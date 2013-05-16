/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#include <paludis/set_file.hh>

#include <paludis/util/log.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/system.hh>
#include <paludis/util/safe_ofstream.hh>
#include <paludis/util/process.hh>
#include <paludis/util/env_var_names.hh>

#include <paludis/environment.hh>
#include <paludis/selection.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/partially_made_package_dep_spec.hh>
#include <paludis/metadata_key.hh>

#include <list>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace paludis;

#include <paludis/set_file-se.cc>

SetFileError::SetFileError(const FSPath & f, const std::string & m) throw () :
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

            virtual std::shared_ptr<SetSpecTree> contents() const = 0;
            virtual bool add(const std::string &) = 0;
            virtual bool remove(const std::string &) = 0;
            virtual void rewrite() const = 0;
    };

    class PaludisConfHandler :
        public SetFileHandler
    {
        private:
            mutable std::mutex _mutex;

            const SetFileParams _p;
            std::list<std::string> _lines;
            mutable std::shared_ptr<SetSpecTree> _contents;

            void _create_contents() const;

        public:
            PaludisConfHandler(const SetFileParams &);

            virtual std::shared_ptr<SetSpecTree> contents() const;
            virtual bool add(const std::string &);
            virtual bool remove(const std::string &);
            virtual void rewrite() const;
    };

    class PaludisBashHandler :
        public SetFileHandler
    {
        private:
            const SetFileParams _p;
            std::shared_ptr<SetSpecTree> _contents;

        public:
            PaludisBashHandler(const SetFileParams &);

            virtual std::shared_ptr<SetSpecTree> contents() const;
            virtual bool add(const std::string &) PALUDIS_ATTRIBUTE((noreturn));
            virtual bool remove(const std::string &) PALUDIS_ATTRIBUTE((noreturn));
            virtual void rewrite() const PALUDIS_ATTRIBUTE((noreturn));
    };

    class SimpleHandler :
        public SetFileHandler
    {
        private:
            mutable std::mutex _mutex;

            const SetFileParams _p;
            std::list<std::string> _lines;
            mutable std::shared_ptr<SetSpecTree> _contents;

            void _create_contents() const;

        public:
            SimpleHandler(const SetFileParams &);

            virtual std::shared_ptr<SetSpecTree> contents() const;
            virtual bool add(const std::string &);
            virtual bool remove(const std::string &);
            virtual void rewrite() const;
    };

    std::shared_ptr<SetFileHandler>
    make_handler(const SetFileParams & p)
    {
        Context context("When making SetFileHandler for '" + stringify(p.file_name()) + "':");

        switch (p.type())
        {
            case sft_simple:
                return std::make_shared<SimpleHandler>(p);

            case sft_paludis_conf:
                return std::make_shared<PaludisConfHandler>(p);

            case sft_paludis_bash:
                return std::make_shared<PaludisBashHandler>(p);

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
            tokenise_whitespace(l, std::back_inserter(tokens));

            return (tokens.size() >= 1) && (tokens.at(1) == query);
        }
    };

    void
    do_one_conf_line(const std::string & line, std::shared_ptr<SetSpecTree> result,
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
            tokenise_whitespace(line, std::back_inserter(tokens));

            if (tokens.empty())
                return;

            if (tokens.size() == 1)
            {
                Log::get_instance()->message("set_file.no_operator", ll_warning, lc_context)
                    << "Line '" << line << "' should start with '?' or '*', assuming '*'";
                tokens.insert(tokens.begin(), "*");
            }

            if ("*" == tokens.at(0) || ((sfsmo_star == params.set_operator_mode()) &&
                        ("?" == tokens.at(0) || "?:" == tokens.at(0))))
            {
                if (std::string::npos == tokens.at(1).find('/'))
                {
                    std::shared_ptr<NamedSetDepSpec> spec;
                    switch (params.set_operator_mode())
                    {
                        case sfsmo_natural:
                            spec = std::make_shared<NamedSetDepSpec>(SetName(tokens.at(1)));
                            break;

                        case sfsmo_star:
                            {
                                std::pair<SetName, SetFileSetOperatorMode> s(find_base_set_name_and_suffix_mode(SetName(tokens.at(1))));
                                spec = std::make_shared<NamedSetDepSpec>(SetName(stringify(s.first) + "*"));
                            }
                            break;

                        case last_sfsmo:
                            break;
                    }
                    if (! spec)
                        throw InternalError(PALUDIS_HERE, "Bad params.set_name_suffix");

                    result->top()->append(spec);
                }
                else
                {
                    std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(params.parser()(tokens.at(1))));
                    result->top()->append(spec);
                }
            }
            else if ("?" == tokens.at(0))
            {
                if (std::string::npos == tokens.at(1).find('/'))
                {
                    Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                        << "? operator may not be used with a set name";
                    return;
                }

                std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(params.parser()(tokens.at(1))));
                if (spec->package_ptr())
                {
                    if (! params.environment())
                        Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                            << "Line '" << line << "' uses ? operator but no environment is available";
                    else if (! (*params.environment())[selection::SomeArbitraryVersion(
                                generator::Package(*spec->package_ptr()) |
                                filter::InstalledAtRoot(params.environment()->preferred_root_key()->parse_value()))]->empty())
                        result->top()->append(spec);
                }
                else
                    Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                        << "Line '" << line << "' uses ? operator but does not specify an unambiguous package";
            }
            else if ("?:" == tokens.at(0))
            {
                if (std::string::npos == tokens.at(1).find('/'))
                {
                    Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                        << "?: operator may not be used with a set name";
                    return;
                }

                std::shared_ptr<PackageDepSpec> spec(std::make_shared<PackageDepSpec>(params.parser()(tokens.at(1))));
                if (spec->package_ptr())
                {
                    if (! params.environment())
                        Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                            << "Line '" << line << "' uses ?: operator but no environment is available";
                    else if (! (*params.environment())[selection::SomeArbitraryVersion(generator::Matches(
                                    make_package_dep_spec({ })
                                    .package(*spec->package_ptr())
                                    .slot_requirement(spec->slot_requirement_ptr()),
                                    nullptr, { }) |
                                filter::InstalledAtRoot(params.environment()->preferred_root_key()->parse_value()))]->empty())
                        result->top()->append(spec);
                }
                else
                    Log::get_instance()->message("set_file.bad_operator", ll_warning, lc_context)
                        << "Line '" << line << "' uses ? operator but does not specify an unambiguous package";
            }
            else
                Log::get_instance()->message("set_file.ignoring_line", ll_warning, lc_context)
                    << "Ignoring line '" << line << "' because it does not start with '?' or '*'";
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("set_file.ignoring_line", ll_warning, lc_context)
                << "Ignoring line '" << line << "' due to exception '" << e.message() << "' (" << e.what() << ")";
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
    Context context("When loading simple set file '" + stringify(_p.file_name()) + "':");

    LineConfigFile ff(_p.file_name(), { lcfo_disallow_continuations, lcfo_disallow_comments, lcfo_no_skip_blank_lines });
    for (LineConfigFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        _lines.push_back(*line);
}

void
SimpleHandler::_create_contents() const
{
    Context context("When parsing atoms in simple set file '" + stringify(_p.file_name()) + "':");

    _contents = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
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
            if (std::string::npos == i->find('/'))
            {
                std::shared_ptr<NamedSetDepSpec> p(std::make_shared<NamedSetDepSpec>(SetName(*i)));
                _contents->top()->append(p);
            }
            else
            {
                std::shared_ptr<PackageDepSpec> p(std::make_shared<PackageDepSpec>(_p.parser()(stringify(*i))));
                _contents->top()->append(p);
            }
        }
        catch (const InternalError &)
        {
            throw;
        }
        catch (const Exception & e)
        {
            Log::get_instance()->message("set_file.ignoring_line", ll_warning, lc_context)
                << "Ignoring line '" << *i << "' due to exception '" << e.message() << "' (" << e.what() << "'";
        }
    }
}

std::shared_ptr<SetSpecTree>
SimpleHandler::contents() const
{
    std::unique_lock<std::mutex> l(_mutex);

    if (! _contents)
        _create_contents();

    return _contents;
}

bool
SimpleHandler::add(const std::string & p)
{
    std::unique_lock<std::mutex> l(_mutex);

    bool result(false);
    if (_lines.end() == std::find(_lines.begin(), _lines.end(), p))
    {
        result = true;
        _lines.push_back(p);
    }

    _contents.reset();

    return result;
}

bool
SimpleHandler::remove(const std::string & p)
{
    std::unique_lock<std::mutex> l(_mutex);

    Context context("When removing '" + stringify(p) + "' from simple set file '" + stringify(_p.file_name()) + "':");

    _contents.reset();

    bool result(false);
    for (std::list<std::string>::iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; )
        if (*i == p)
        {
            result = true;
            _lines.erase(i++);
        }
        else
            ++i;

    return result;
}

void
SimpleHandler::rewrite() const
{
    std::unique_lock<std::mutex> l(_mutex);

    Context context("When rewriting simple set file '" + stringify(_p.file_name()) + "':");

    try
    {
        SafeOFStream f(_p.file_name(), -1, true);

        for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
                i != i_end ; ++i)
            f << *i << std::endl;
    }
    catch (const SafeOFStreamError & e)
    {
        throw SetFileError(_p.file_name(), "Cannot write to '" + stringify(_p.file_name()) + "': '" + e.message() + "' ("
                + e.what() + ")");
    }
}

PaludisConfHandler::PaludisConfHandler(const SetFileParams & p) :
    _p(p)
{
    Context context("When loading paludis conf set file '" + stringify(_p.file_name()) + "':");

    LineConfigFile ff(_p.file_name(), { lcfo_disallow_continuations, lcfo_disallow_comments, lcfo_no_skip_blank_lines });
    for (LineConfigFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        _lines.push_back(*line);
}

void
PaludisConfHandler::_create_contents() const
{
    Context context("When parsing atoms in paludis conf set file '" + stringify(_p.file_name()) + "':");

    _contents = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
    for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; ++i)
        do_one_conf_line(*i, _contents, _p);
}

std::shared_ptr<SetSpecTree>
PaludisConfHandler::contents() const
{
    std::unique_lock<std::mutex> l(_mutex);

    if (! _contents)
        _create_contents();

    return _contents;
}

bool
PaludisConfHandler::add(const std::string & p)
{
    std::unique_lock<std::mutex> l(_mutex);

    bool result(false);
    if (_lines.end() == std::find_if(_lines.begin(), _lines.end(), TokenOneIs(p)))
    {
        result = true;
        _lines.push_back("* " + p);
    }

    _contents.reset();
    return result;
}

bool
PaludisConfHandler::remove(const std::string & p)
{
    Context context("When removing '" + stringify(p) + "' from paludis conf set file '" + stringify(_p.file_name()) + "':");

    std::unique_lock<std::mutex> l(_mutex);
    _contents.reset();

    bool result(false);
    for (std::list<std::string>::iterator i(_lines.begin()), i_end(_lines.end()) ;
            i != i_end ; )
        if (TokenOneIs(p)(*i))
        {
            result = true;
            _lines.erase(i++);
        }
        else
            ++i;

    return result;
}

void
PaludisConfHandler::rewrite() const
{
    Context context("When rewriting paludis conf set file '" + stringify(_p.file_name()) + "':");

    std::unique_lock<std::mutex> l(_mutex);

    try
    {
        SafeOFStream f(_p.file_name(), -1, true);

        for (std::list<std::string>::const_iterator i(_lines.begin()), i_end(_lines.end()) ;
                i != i_end ; ++i)
            f << *i << std::endl;
    }
    catch (const SafeOFStreamError & e)
    {
        throw SetFileError(_p.file_name(), "Cannot write to '" + stringify(_p.file_name()) + "': '" + e.message() + "' ("
                + e.what() + ")");
    }
}

PaludisBashHandler::PaludisBashHandler(const SetFileParams & p) :
    _p(p)
{
    Context context("When loading paludis bash set file '" + stringify(_p.file_name()) + "':");
    _contents = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());

    std::stringstream s;
    Process process(ProcessCommand({ "bash", stringify(_p.file_name()) }));
    process
        .setenv("ROOT", _p.environment() ? stringify(_p.environment()->preferred_root_key()->parse_value()) : "/")
        .setenv("SET", stringify(_p.file_name()))
        .setenv("SET_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .prefix_stderr(_p.file_name().basename() + "> ")
        .capture_stdout(s);
    int exit_status(process.run().wait());

    LineConfigFile ff(s, { lcfo_disallow_continuations, lcfo_disallow_comments, lcfo_no_skip_blank_lines });
    for (LineConfigFile::ConstIterator line(ff.begin()), line_end(ff.end()) ;
            line != line_end ; ++line)
        do_one_conf_line(*line, _contents, _p);

    if (exit_status != 0)
    {
        Log::get_instance()->message("set_file.script.failure", ll_warning, lc_context)
            << "Set file script '" << _p.file_name() << "' returned non-zero exit status '"
            << exit_status << "'";
        _contents = std::make_shared<SetSpecTree>(std::make_shared<AllDepSpec>());
    }
}

std::shared_ptr<SetSpecTree>
PaludisBashHandler::contents() const
{
    return _contents;
}

bool
PaludisBashHandler::add(const std::string & p)
{
    throw SetFileError(_p.file_name(), "Cannot add entry '" + p + "' to bash script '" + stringify(_p.file_name()) + "'");
}

bool
PaludisBashHandler::remove(const std::string & p)
{
    throw SetFileError(_p.file_name(), "Cannot remove entry '" + p + "' from bash script '" + stringify(_p.file_name()) + "'");
}

void
PaludisBashHandler::rewrite() const
{
    throw SetFileError(_p.file_name(), "Cannot modify bash script '" + stringify(_p.file_name()) + "'");
}

namespace paludis
{
    template<>
    struct Imp<SetFile>
    {
        const SetFileParams params;
        std::shared_ptr<SetFileHandler> handler;

        Imp(const SetFileParams & p) :
            params(p),
            handler(make_handler(p))
        {
        }
    };
}

SetFile::SetFile(const SetFileParams & p) :
    _imp(p)
{
}

SetFile::~SetFile()
{
}

const std::shared_ptr<const SetSpecTree>
SetFile::contents() const
{
    return _imp->handler->contents();
}

void
SetFile::rewrite() const
{
    _imp->handler->rewrite();
}

bool
SetFile::add(const std::string & p)
{
    return _imp->handler->add(p);
}

bool
SetFile::remove(const std::string & p)
{
    return _imp->handler->remove(p);
}

std::pair<SetName, SetFileSetOperatorMode>
paludis::find_base_set_name_and_suffix_mode(const SetName & s)
{
    Context context("When working out whether '" + stringify(s) + "' has operators:");
    std::string ss(stringify(s));
    if (ss.length() >= 2 && '*' == ss[ss.length() - 1])
        return std::make_pair(SetName(ss.substr(0, ss.length() - 1)), sfsmo_star);
    else
        return std::make_pair(s, sfsmo_natural);
}

