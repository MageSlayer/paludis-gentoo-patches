/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/ndbam_unmerger.hh>
#include <paludis/ndbam.hh>

#include <paludis/util/destringify.hh>
#include <paludis/util/md5.hh>
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/package_database.hh>
#include <paludis/package_id.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

using namespace paludis;

#include <paludis/ndbam_unmerger-sr.cc>

namespace paludis
{
    template<>
    struct Implementation<NDBAMUnmerger>
    {
        NDBAMUnmergerOptions options;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Implementation(const NDBAMUnmergerOptions & o) :
            options(o)
        {
            tokenise_whitespace(o.config_protect, std::back_inserter(config_protect));
            tokenise_whitespace(o.config_protect_mask, std::back_inserter(config_protect_mask));
        }
    };
}

class NDBAMUnmerger::FileExtraInfo :
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

class NDBAMUnmerger::SymlinkExtraInfo :
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

NDBAMUnmerger::NDBAMUnmerger(const NDBAMUnmergerOptions & o) :
    Unmerger(UnmergerOptions::create()
            .environment(o.environment)
            .root(o.root)),
    PrivateImplementationPattern<NDBAMUnmerger>(new Implementation<NDBAMUnmerger>(o)),
    _imp(PrivateImplementationPattern<NDBAMUnmerger>::_imp.get())
{
}

NDBAMUnmerger::~NDBAMUnmerger()
{
}

Hook
NDBAMUnmerger::extend_hook(const Hook & h) const
{
    tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->options.environment->bashrc_files());

    Hook result(Unmerger::extend_hook(h)
        ("CONFIG_PROTECT", _imp->options.config_protect)
        ("CONFIG_PROTECT_MASK", _imp->options.config_protect_mask)
        ("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (_imp->options.package_id)
    {
        std::string cat(stringify(_imp->options.package_id->name().category));
        std::string pn(stringify(_imp->options.package_id->name().package));
        std::string pvr(stringify(_imp->options.package_id->version()));
        std::string pv(stringify(_imp->options.package_id->version().remove_revision()));

        return result
            ("P", pn + "-" + pv)
            ("PN", pn)
            ("CATEGORY", cat)
            ("PR", _imp->options.package_id->version().revision_only())
            ("PV", pv)
            ("PVR", pvr)
            ("PF", pn + "-" + pvr)
            ("SLOT", stringify(_imp->options.package_id->slot()));
    }

    return result;
}

bool
NDBAMUnmerger::config_protected(const FSEntry & f) const
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
NDBAMUnmerger::make_tidy(const FSEntry & f) const
{
    std::string root_str(stringify(_imp->options.root)), f_str(stringify(f));
    if (root_str == "/")
        root_str.clear();
    if (0 != f_str.compare(0, root_str.length(), root_str))
        throw NDBAMUnmergerError("Can't work out tidy name for '" + f_str + "' with root '" + root_str + "'");
    return f_str.substr(root_str.length());
}

void
NDBAMUnmerger::_add_file(const FSEntry & f, const std::string & md5, const time_t mtime)
{
    add_unmerge_entry(stringify(f), et_file, make_shared_ptr(new FileExtraInfo(md5, mtime)));
}

void
NDBAMUnmerger::_add_dir(const FSEntry & f)
{
    add_unmerge_entry(stringify(f), et_dir, tr1::shared_ptr<ExtraInfo>());
}

void
NDBAMUnmerger::_add_sym(const FSEntry & f, const std::string & target, const time_t mtime)
{
    add_unmerge_entry(stringify(f), et_sym, make_shared_ptr(new SymlinkExtraInfo(target, mtime)));
}

void
NDBAMUnmerger::populate_unmerge_set()
{
    using namespace tr1::placeholders;
    _imp->options.ndbam->parse_contents(*_imp->options.package_id,
            tr1::bind(&NDBAMUnmerger::_add_file, this, _1, _2, _3),
            tr1::bind(&NDBAMUnmerger::_add_dir, this, _1),
            tr1::bind(&NDBAMUnmerger::_add_sym, this, _1, _2, _3)
            );
}

bool
NDBAMUnmerger::check_file(const FSEntry & f, tr1::shared_ptr<ExtraInfo> ei) const
{
    tr1::shared_ptr<FileExtraInfo> fie(tr1::static_pointer_cast<FileExtraInfo>(ei));

    if (! (_imp->options.root / f).is_regular_file())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root / f).mtime() != fie->_mtime)
        display("--- [!time] " + stringify(f));
    else
    {
        std::ifstream md5_file(stringify(_imp->options.root / f).c_str());
        if (! md5_file)
        {
            Log::get_instance()->message(ll_warning, lc_no_context, "Cannot get md5 for '" +
                    stringify(_imp->options.root / f) + "'");
            display("--- [!md5?] " + stringify(f));
        }
        else if (MD5(md5_file).hexsum() != fie->_md5sum)
            display("--- [!md5 ] " + stringify(f));
        else if (config_protected(_imp->options.root / f))
            display("--- [cfgpr] " + stringify(f));
        else
            return true;
    }

    return false;
}

bool
NDBAMUnmerger::check_sym(const FSEntry & f, tr1::shared_ptr<ExtraInfo> ei) const
{
    tr1::shared_ptr<SymlinkExtraInfo> sie(tr1::static_pointer_cast<SymlinkExtraInfo>(ei));

    if (! (_imp->options.root / f).is_symbolic_link())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root / f).mtime() != sie->_mtime)
        display("--- [!time] " + stringify(f));
    else if ((_imp->options.root / f).readlink() != sie->_dest)
        display("--- [!dest] " + stringify(f));
    else
        return true;

    return false;
}

bool
NDBAMUnmerger::check_misc(const FSEntry &, tr1::shared_ptr<ExtraInfo>) const
{
    return false;
}

bool
NDBAMUnmerger::check_dir(const FSEntry & f, tr1::shared_ptr<ExtraInfo>) const
{
    if (! (_imp->options.root / f).is_directory())
        display("--- [!type] " + stringify(f));
    else if (DirIterator(_imp->options.root / f, DirIteratorOptions() + dio_include_dotfiles + dio_first_only) != DirIterator())
        display("--- [!empt] " + stringify(f));
    else
        return true;

    return false;
}

void
NDBAMUnmerger::display(const std::string & message) const
{
    std::cout << message << std::endl;
}

NDBAMUnmergerError::NDBAMUnmergerError(const std::string & s) throw () :
    UnmergerError(s)
{
}

