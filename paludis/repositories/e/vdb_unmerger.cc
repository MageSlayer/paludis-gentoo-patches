/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#include "vdb_unmerger.hh"
#include "vdb_contents_tokeniser.hh"

using namespace paludis;

#include <paludis/util/destringify.hh>
#include <paludis/util/md5.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/make_named_values.hh>

#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

namespace paludis
{
    template<>
    struct Implementation<VDBUnmerger>
    {
        VDBUnmergerOptions options;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Implementation(const VDBUnmergerOptions & o) :
            options(o)
        {
            tokenise_whitespace(o.config_protect(), std::back_inserter(config_protect));
            tokenise_whitespace(o.config_protect_mask(), std::back_inserter(config_protect_mask));
        }
    };
}

class VDBUnmerger::FileExtraInfo :
    public Unmerger::ExtraInfo
{
    public:
        std::string _md5sum;
        time_t _mtime;

        FileExtraInfo(std::string md5sum, time_t mtime) :
            _md5sum(md5sum),
            _mtime(mtime)
        {
        }

        virtual ~FileExtraInfo()
        {
        }
};

class VDBUnmerger::SymlinkExtraInfo :
    public Unmerger::ExtraInfo
{
    public:
        std::string _dest;
        time_t _mtime;

        SymlinkExtraInfo(std::string dest, time_t mtime) :
            _dest(dest),
            _mtime(mtime)
        {
        }

        virtual ~SymlinkExtraInfo()
        {
        }
};

class VDBUnmerger::MiscExtraInfo :
    public Unmerger::ExtraInfo
{
    public:
        std::string _type;

        MiscExtraInfo(std::string type) :
            _type(type)
        {
        }

        virtual ~MiscExtraInfo()
        {
        }
};

VDBUnmerger::VDBUnmerger(const VDBUnmergerOptions & o) :
    Unmerger(make_named_values<UnmergerOptions>(
                value_for<n::environment>(o.environment()),
                value_for<n::root>(o.root())
            )),
    PrivateImplementationPattern<VDBUnmerger>(new Implementation<VDBUnmerger>(o)),
    _imp(PrivateImplementationPattern<VDBUnmerger>::_imp.get())
{
}

VDBUnmerger::~VDBUnmerger()
{
}

Hook
VDBUnmerger::extend_hook(const Hook & h) const
{
    std::tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->options.environment()->bashrc_files());

    Hook result(Unmerger::extend_hook(h)
        ("CONFIG_PROTECT", _imp->options.config_protect())
        ("CONFIG_PROTECT_MASK", _imp->options.config_protect_mask())
        ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (_imp->options.package_id())
    {
        std::string cat(stringify(_imp->options.package_id()->name().category()));
        std::string pn(stringify(_imp->options.package_id()->name().package()));
        std::string pvr(stringify(_imp->options.package_id()->version()));
        std::string pv(stringify(_imp->options.package_id()->version().remove_revision()));
        std::string slot(_imp->options.package_id()->slot_key() ? stringify(_imp->options.package_id()->slot_key()->value()) : "");

        return result
            ("P", pn + "-" + pv)
            ("PNV", pn + "-" + pv)
            ("PN", pn)
            ("CATEGORY", cat)
            ("PR", _imp->options.package_id()->version().revision_only())
            ("PV", pv)
            ("PVR", pvr)
            ("PF", pn + "-" + pvr)
            ("PNVR", pn + "-" + pvr)
            ("SLOT", slot)
            ;
    }

    return result;
}

bool
VDBUnmerger::config_protected(const FSEntry & f) const
{
    std::string tidy(make_tidy(f));

    bool result(false);
    for (std::list<std::string>::const_iterator c(_imp->config_protect.begin()),
            c_end(_imp->config_protect.end()) ; c != c_end && ! result ; ++c)
    {
        std::string cc(strip_trailing(*c, "/") + "/");
        if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
            result = true;
    }
    if (result)
        for (std::list<std::string>::const_iterator c(_imp->config_protect_mask.begin()),
                c_end(_imp->config_protect_mask.end()) ; c != c_end && result ; ++c)
        {
            std::string cc(strip_trailing(*c, "/") + "/");
            if (tidy == *c || 0 == tidy.compare(0, cc.length(), cc))
                result = false;
        }

    return result;
}

std::string
VDBUnmerger::make_tidy(const FSEntry & f) const
{
    std::string root_str(stringify(_imp->options.root())), f_str(stringify(f));
    if (root_str == "/")
        root_str.clear();
    if (0 != f_str.compare(0, root_str.length(), root_str))
        throw VDBUnmergerError("Can't work out tidy name for '" + f_str + "' with root '" + root_str + "'");
    return f_str.substr(root_str.length());
}

void
VDBUnmerger::populate_unmerge_set()
{
    std::ifstream c(stringify(_imp->options.contents_file()).c_str());
    if (! c)
        throw VDBUnmergerError("Cannot read '" + stringify(_imp->options.contents_file()) + "'");

    std::string line;
    while (std::getline(c, line))
    {
        std::vector<std::string> tokens;
        if (! erepository::VDBContentsTokeniser::tokenise(line, std::back_inserter(tokens)))
        {
            Log::get_instance()->message("e.vdb.contents.malformed", ll_warning, lc_no_context) << "Malformed VDB entry '" << line << "'";
            continue;
        }

        if ("obj" == tokens.at(0))
        {
            std::string md5sum(tokens.at(2));
            time_t mtime(destringify<time_t>(tokens.at(3)));
            std::tr1::shared_ptr<ExtraInfo> extra(new FileExtraInfo(md5sum, mtime));
            add_unmerge_entry(tokens.at(1), et_file, extra);

        }
        else if ("sym" == tokens.at(0))
        {
            std::string dest(tokens.at(2));
            time_t mtime(destringify<time_t>(tokens.at(3)));
            std::tr1::shared_ptr<ExtraInfo> extra(new SymlinkExtraInfo(dest, mtime));
            add_unmerge_entry(tokens.at(1), et_sym, extra);
        }
        else if ("misc" == tokens.at(0))
        {
        }
        else if ("fif" == tokens.at(0) || "dev" == tokens.at(0))
        {
            std::string type(tokens.at(0));
            std::tr1::shared_ptr<ExtraInfo> extra(new MiscExtraInfo(type));
            add_unmerge_entry(tokens.at(1), et_misc, extra);
        }
        else if ("dir" == tokens.at(0))
        {
            add_unmerge_entry(tokens.at(1), et_dir, std::tr1::shared_ptr<ExtraInfo>());
        }
        else
            Log::get_instance()->message("e.vdb.contents.malformed", ll_warning, lc_no_context)
                << "Malformed VDB entry '" << line << "'";
    }
}

bool
VDBUnmerger::check_file(const FSEntry & f, const std::tr1::shared_ptr<ExtraInfo> & ei) const
{
    std::tr1::shared_ptr<FileExtraInfo> fie(std::tr1::static_pointer_cast<FileExtraInfo>(ei));

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_regular_file())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root() / f).mtime() != fie->_mtime)
        display("--- [!time] " + stringify(f));
    else
    {
        std::ifstream md5_file(stringify(_imp->options.root() / f).c_str());
        if (! md5_file)
        {
            Log::get_instance()->message("e.vdb.contents.md5_failed", ll_warning, lc_no_context)
                << "Cannot get md5 for '" << (_imp->options.root() / f) << "'";
            display("--- [!md5?] " + stringify(f));
        }
        else if (MD5(md5_file).hexsum() != fie->_md5sum)
            display("--- [!md5 ] " + stringify(f));
        else if (config_protected(_imp->options.root() / f))
            display("--- [cfgpr] " + stringify(f));
        else
            return true;
    }

    return false;
}

bool
VDBUnmerger::check_sym(const FSEntry & f, const std::tr1::shared_ptr<ExtraInfo> & ei) const
{
    std::tr1::shared_ptr<SymlinkExtraInfo> sie(std::tr1::static_pointer_cast<SymlinkExtraInfo>(ei));

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_symbolic_link())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root() / f).mtime() != sie->_mtime)
        display("--- [!time] " + stringify(f));
    else if ((_imp->options.root() / f).readlink() != sie->_dest)
        display("--- [!dest] " + stringify(f));
    else
        return true;

    return false;
}

bool
VDBUnmerger::check_misc(const FSEntry & f, const std::tr1::shared_ptr<ExtraInfo> & ei) const
{
    std::tr1::shared_ptr<MiscExtraInfo> mie(std::tr1::static_pointer_cast<MiscExtraInfo>(ei));

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if ("fif" == mie->_type && ! (_imp->options.root() / f).is_fifo())
        display("--- [!type] " + stringify(f));
    else if ("dev" == mie->_type && ! (_imp->options.root() / f).is_device())
        display("--- [!type] " + stringify(f));
    else
        return true;

    return false;
}

bool
VDBUnmerger::check_dir(const FSEntry & f, const std::tr1::shared_ptr<ExtraInfo> &) const
{
    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_directory())
        display("--- [!type] " + stringify(f));
    else if (DirIterator(_imp->options.root() / f, DirIteratorOptions() + dio_include_dotfiles + dio_first_only) != DirIterator())
        display("--- [!empt] " + stringify(f));
    else
        return true;

    return false;
}

void
VDBUnmerger::display(const std::string & message) const
{
    std::cout << message << std::endl;
}

VDBUnmergerError::VDBUnmergerError(const std::string & s) throw () :
    UnmergerError(s)
{
}

