/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "set_file.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pstream.hh>
#include <paludis/config_file.hh>
#include <paludis/environment.hh>
#include <paludis/query.hh>
#include <list>
#include <vector>
#include <fstream>

using namespace paludis;

#include <paludis/set_file-se.cc>
#include <paludis/set_file-sr.cc>

SetFileError::SetFileError(const FSEntry & f, const std::string & m) throw () :
    ConfigurationError("In set file '" + stringify(f) + "': " + m)
{
}

namespace
{
    class SetFileHandler
    {
        protected:
            SetFileHandler();

        public:
            virtual ~SetFileHandler();

            virtual std::tr1::shared_ptr<CompositeDepSpec> contents() const = 0;
            virtual void add(const std::string &) = 0;
            virtual void remove(const std::string &) = 0;
            virtual void rewrite() const = 0;
    };

    class PaludisConfHandler :
        public SetFileHandler
    {
        private:
            const SetFileParams _p;
            std::list<std::string> _lines;
            mutable std::tr1::shared_ptr<CompositeDepSpec> _contents;

            void _create_contents() const;

        public:
            PaludisConfHandler(const SetFileParams &);

            virtual std::tr1::shared_ptr<CompositeDepSpec> contents() const;
            virtual void add(const std::string &);
            virtual void remove(const std::string &);
            virtual void rewrite() const;
    };

    class PaludisBashHandler :
        public SetFileHandler
    {
        private:
            const SetFileParams _p;
            std::tr1::shared_ptr<CompositeDepSpec> _contents;

        public:
            PaludisBashHandler(const SetFileParams &);

            virtual std::tr1::shared_ptr<CompositeDepSpec> contents() const;
            virtual void add(const std::string &) PALUDIS_ATTRIBUTE((noreturn));
            virtual void remove(const std::string &) PALUDIS_ATTRIBUTE((noreturn));
            virtual void rewrite() const PALUDIS_ATTRIBUTE((noreturn));
    };

    class SimpleHandler :
        public SetFileHandler
    {
        private:
            const SetFileParams _p;
            std::list<std::string> _lines;
            mutable std::tr1::shared_ptr<CompositeDepSpec> _contents;

            void _create_contents() const;

        public:
            SimpleHandler(const SetFileParams &);

            virtual std::tr1::shared_ptr<CompositeDepSpec> contents() const;
            virtual void add(const std::string &);
            virtual void remove(const std::string &);
            virtual void rewrite() const;
    };

    std::tr1::shared_ptr<SetFileHandler>
    make_handler(const SetFileParams & p)
    {
        Context context("When making SetFileHandler for '" + stringify(p.file_name) + "':");

        switch (p.type)
        {
            case sft_simple:
                return std::tr1::shared_ptr<SetFileHandler>(new SimpleHandler(p));

            case sft_paludis_conf:
                return std::tr1::shared_ptr<SetFileHandler>(new PaludisConfHandler(p));

            case sft_paludis_bash:
                return std::tr1::shared_ptr<SetFileHandler>(new PaludisBashHandler(p));

            case last_sft:
                break;
        }

        throw InternalError(PALUDIS_HERE, "Bad SetFileType");
    }

    struct TokenOneIs
    {
        const std::string query;

        TokenOneIs(const std::string & q) :
            query(q)
        {
        }

        bool operator() (const std::string & l) const
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(l, std::back_inserter(tokens));

            return (tokens.size() >= 1) && (tokens.at(1) == query);
        }
    };

    void
    do_one_conf_line(const std::string & line, std::tr1::shared_ptr<CompositeDepSpec> result,
            const SetFileParams & params)
    {
        if (line.empty())
            return;

        if ('#' == line.at(0))
            return;

        Context c("When handling line '" + stringify(line) + "':");

        try
        {
            std::vector<std::string> tokens;
            WhitespaceTokeniser::get_instance()->tokenise(line, std::back_inserter(tokens));

            if (tokens.empty())
                return;

            if (tokens.size() == 1)
            {
                Log::get_instance()->message(ll_warning, lc_context, "Line '" + stringify(line) +
                        "' should start with '?' or '*', assuming '*'");

                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(tokens.at(0), params.parse_mode));
                if (params.tag)
                    spec->set_tag(params.tag);
                result->add_child(spec);
            }
            else if ("*" == tokens.at(0))
            {
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(tokens.at(1), params.parse_mode));
                if (params.tag)
                    spec->set_tag(params.tag);
                result->add_child(spec);
            }
            else if ("?" == tokens.at(0))
            {
                std::tr1::shared_ptr<PackageDepSpec> spec(new PackageDepSpec(tokens.at(1), params.parse_mode));
                if (params.tag)
                    spec->set_tag(params.tag);

                if (spec->package_ptr())
                {
                    if (! params.environment)
                        Log::get_instance()->message(ll_warning, lc_context, "Line '" + stringify(line) +
                                "' uses ? operator but no environment is available");
                    else if (! params.environment->package_database()->query(query::Package(*spec->package_ptr()) &
                                query::InstalledAtRoot(params.environment->root()), qo_whatever)->empty())
                        result->add_child(spec);
                }
                else
                    Log::get_instance()->message(ll_warning, lc_context, "Line '" + stringify(line) +
                            "' uses ? operator but does not specify an unambiguous package");
            }
            else
                Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '" + stringify(line) +
                        "' because it does not start with '?' or '*'");
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '" + stringify(line) +
                    "' due to exception '" + e.message() + "' (" + e.what() + ")");
        }
    }
}

SetFileHandler::SetFileHandler()
{
}

SetFileHandler::~SetFileHandler()
{
}

SimpleHandler::SimpleHandler(const SetFileParams & p) :
    _p(p)
{
    Context context("When loading simple set file '" + stringify(_p.file_name) + "':");

    LineConfigFile ff(_p.file_name, LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments
            + lcfo_no_skip_blank_lines);
    for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        _lines.push_back(*line);
}

void
SimpleHandler::_create_contents() const
{
    Context context("When parsing atoms in simple set file '" + stringify(_p.file_name) + "':");

    _contents.reset(new AllDepSpec);
    for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; ++i)
    {
        if (i->empty())
            continue;

        if ('#' == i->at(0))
            continue;

        Context c("When handling line '" + stringify(*i) + "':");

        try
        {
            if (_p.environment && std::string::npos == i->find('/'))
            {
                std::tr1::shared_ptr<DepSpec> p(_p.environment->package_set(SetName(*i)));
                if (p)
                    _contents->add_child(p);
                else
                    Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '" + stringify(*i) +
                            "' because it does not contain a known set name");
            }
            else
            {
                std::tr1::shared_ptr<PackageDepSpec> p(new PackageDepSpec(stringify(*i), _p.parse_mode));
                if (_p.tag)
                    p->set_tag(_p.tag);
                _contents->add_child(p);
            }
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message(ll_warning, lc_context, "Ignoring line '" + stringify(*i) +
                    "' due to exception '" + e.message() + "' (" + e.what() + "'");
        }
    }
}

std::tr1::shared_ptr<CompositeDepSpec>
SimpleHandler::contents() const
{
    if (! _contents)
        _create_contents();

    return _contents;
}

void
SimpleHandler::add(const std::string & p)
{
    if (_lines.end() == std::find(_lines.begin(), _lines.end(), p))
        _lines.push_back(p);

    _contents.reset();
}

void
SimpleHandler::remove(const std::string & p)
{
    Context context("When removing '" + stringify(p) + "' from simple set file '" + stringify(_p.file_name) + "':");

    _contents.reset();
    _lines.remove(p);
}

void
SimpleHandler::rewrite() const
{
    Context context("When rewriting simple set file '" + stringify(_p.file_name) + "':");

    std::ofstream f(stringify(_p.file_name).c_str());
    if (! f)
        throw SetFileError(_p.file_name, "Cannot write to '" + stringify(_p.file_name) + "'");

    for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; ++i)
        f << *i << std::endl;
}

PaludisConfHandler::PaludisConfHandler(const SetFileParams & p) :
    _p(p)
{
    Context context("When loading paludis conf set file '" + stringify(_p.file_name) + "':");

    LineConfigFile ff(_p.file_name, LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments
            + lcfo_no_skip_blank_lines);
    for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        _lines.push_back(*line);
}

void
PaludisConfHandler::_create_contents() const
{
    Context context("When parsing atoms in paludis conf set file '" + stringify(_p.file_name) + "':");

    _contents.reset(new AllDepSpec);
    for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; ++i)
        do_one_conf_line(*i, _contents, _p);
}

std::tr1::shared_ptr<CompositeDepSpec>
PaludisConfHandler::contents() const
{
    if (! _contents)
        _create_contents();

    return _contents;
}

void
PaludisConfHandler::add(const std::string & p)
{
    if (_lines.end() == std::find_if(_lines.begin(), _lines.end(), TokenOneIs(p)))
        _lines.push_back("* " + p);

    _contents.reset();
}

void
PaludisConfHandler::remove(const std::string & p)
{
    Context context("When removing '" + stringify(p) + "' from paludis conf set file '" + stringify(_p.file_name) + "':");

    _contents.reset();
    _lines.remove_if(TokenOneIs(p));
}

void
PaludisConfHandler::rewrite() const
{
    Context context("When rewriting paludis conf set file '" + stringify(_p.file_name) + "':");

    std::ofstream f(stringify(_p.file_name).c_str());
    if (! f)
        throw SetFileError(_p.file_name, "Cannot write to '" + stringify(_p.file_name) + "'");

    for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; ++i)
        f << *i << std::endl;
}

PaludisBashHandler::PaludisBashHandler(const SetFileParams & p) :
    _p(p)
{
    Context context("When loading paludis bash set file '" + stringify(_p.file_name) + "':");
    _contents.reset(new AllDepSpec);

    Command cmd(Command("bash '" + stringify(_p.file_name) + "'")
            .with_setenv("ROOT", _p.environment ? stringify(_p.environment->root()) : "/")
            .with_setenv("SET", stringify(_p.file_name))
            .with_setenv("SET_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
            .with_setenv("PALUDIS_COMMAND", _p.environment ? _p.environment->paludis_command() : "")
            .with_stderr_prefix(_p.file_name.basename() + "> "));
    PStream s(cmd);

    LineConfigFile ff(s, LineConfigFileOptions() + lcfo_disallow_continuations + lcfo_disallow_comments
            + lcfo_no_skip_blank_lines);
    for (LineConfigFile::Iterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        do_one_conf_line(*line, _contents, _p);

    if (s.exit_status() != 0)
    {
        Log::get_instance()->message(ll_warning, lc_context, "Set file script '" + stringify(_p.file_name) +
                "' returned non-zero exit status '" + stringify(s.exit_status()) + "'");
        _contents.reset(new AllDepSpec);
    }
}

std::tr1::shared_ptr<CompositeDepSpec>
PaludisBashHandler::contents() const
{
    return _contents;
}

void
PaludisBashHandler::add(const std::string & p)
{
    throw SetFileError(_p.file_name, "Cannot add entry '" + p + "' to bash script '" + stringify(_p.file_name) + "'");
}

void
PaludisBashHandler::remove(const std::string & p)
{
    throw SetFileError(_p.file_name, "Cannot remove entry '" + p + "' from bash script '" + stringify(_p.file_name) + "'");
}

void
PaludisBashHandler::rewrite() const
{
    throw SetFileError(_p.file_name, "Cannot modify bash script '" + stringify(_p.file_name) + "'");
}

namespace paludis
{
    template<>
    struct Implementation<SetFile>
    {
        const SetFileParams params;
        std::tr1::shared_ptr<SetFileHandler> handler;

        Implementation(const SetFileParams & p) :
            params(p),
            handler(make_handler(p))
        {
        }
    };
}

SetFile::SetFile(const SetFileParams & p) :
    PrivateImplementationPattern<SetFile>(new Implementation<SetFile>(p))
{
}

SetFile::~SetFile()
{
}

std::tr1::shared_ptr<CompositeDepSpec>
SetFile::contents() const
{
    return _imp->handler->contents();
}

void
SetFile::rewrite() const
{
    _imp->handler->rewrite();
}

void
SetFile::add(const std::string & p)
{
    _imp->handler->add(p);
}

void
SetFile::remove(const std::string & p)
{
    return _imp->handler->remove(p);
}

