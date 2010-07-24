/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <paludis/util/pimp-impl.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/set.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/output_manager.hh>
#include <paludis/util/safe_ifstream.hh>

#include <list>
#include <map>
#include <vector>

namespace paludis
{
    template<>
    struct Imp<VDBUnmerger>
    {
        VDBUnmergerOptions options;

        std::list<std::string> config_protect;
        std::list<std::string> config_protect_mask;

        Imp(const VDBUnmergerOptions & o) :
            options(o)
        {
            tokenise_whitespace(o.config_protect(), std::back_inserter(config_protect));
            tokenise_whitespace(o.config_protect_mask(), std::back_inserter(config_protect_mask));
        }
    };
}

VDBUnmerger::VDBUnmerger(const VDBUnmergerOptions & o) :
    Unmerger(make_named_values<UnmergerOptions>(
                n::environment() = o.environment(),
                n::ignore() = o.ignore(),
                n::root() = o.root()
            )),
    Pimp<VDBUnmerger>(o),
    _imp(Pimp<VDBUnmerger>::_imp.get())
{
}

VDBUnmerger::~VDBUnmerger()
{
}

Hook
VDBUnmerger::extend_hook(const Hook & h) const
{
    std::shared_ptr<const FSEntrySequence> bashrc_files(_imp->options.environment()->bashrc_files());

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

namespace
{
    struct GetET
    {
        EntryType visit(const ContentsFileEntry &) const
        {
            return et_file;
        }

        EntryType visit(const ContentsDirEntry &) const
        {
            return et_dir;
        }

        EntryType visit(const ContentsSymEntry &) const
        {
            return et_sym;
        }

        EntryType visit(const ContentsOtherEntry &) const
        {
            return et_misc;
        }
    };

    EntryType get_et(const ContentsEntry & e)
    {
        GetET v;
        return e.accept_returning<EntryType>(v);
    }
}

void
VDBUnmerger::populate_unmerge_set()
{
    if (! _imp->options.package_id()->contents_key()->value())
        throw VDBUnmergerError("Id '" + stringify(*_imp->options.package_id()) + "' has no contents key");

    std::shared_ptr<const Contents> contents(_imp->options.package_id()->contents_key()->value());
    for (Contents::ConstIterator c(contents->begin()), c_end(contents->end()) ;
            c != c_end ; ++c)
        add_unmerge_entry(get_et(**c), *c);
}

namespace
{
    template <typename T_>
    const T_ & require_key(const MetadataKeyHolder & h, const std::string & r)
    {
        MetadataKeyHolder::MetadataConstIterator m(h.find_metadata(r));
        if (m == h.end_metadata())
            throw InternalError(PALUDIS_HERE, "Expected key '" + r + "' not found");

        const T_ * const c(simple_visitor_cast<const T_>(**m));
        if (! c)
            throw InternalError(PALUDIS_HERE, "Key '" + r + "' is of wrong type");

        return *c;
    }
}

bool
VDBUnmerger::check_file(const std::shared_ptr<const ContentsEntry> & e) const
{
    const FSEntry f(e->location_key()->value());
    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_regular_file())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root() / f).mtim().seconds() != require_key<MetadataTimeKey>(*e, "mtime").value().seconds())
        display("--- [!time] " + stringify(f));
    else
    {
        try
        {
            SafeIFStream md5_file(_imp->options.root() / f);
            if (MD5(md5_file).hexsum() != require_key<MetadataValueKey<std::string> >(*e, "md5").value())
                display("--- [!md5 ] " + stringify(f));
            else if (config_protected(_imp->options.root() / f))
                display("--- [cfgpr] " + stringify(f));
            else
                return true;
        }
        catch (const SafeIFStreamError &)
        {
            Log::get_instance()->message("e.vdb.contents.md5_failed", ll_warning, lc_no_context)
                << "Cannot get md5 for '" << (_imp->options.root() / f) << "'";
            display("--- [!md5?] " + stringify(f));
        }
    }

    return false;
}

bool
VDBUnmerger::check_sym(const std::shared_ptr<const ContentsEntry> & e) const
{
    const FSEntry f(e->location_key()->value());

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_symbolic_link())
        display("--- [!type] " + stringify(f));
    else if ((_imp->options.root() / f).mtim().seconds() != require_key<MetadataTimeKey>(*e, "mtime").value().seconds())
        display("--- [!time] " + stringify(f));
    else if ((_imp->options.root() / f).readlink() != require_key<MetadataValueKey<std::string> >(*e, "target").value())
        display("--- [!dest] " + stringify(f));
    else
        return true;

    return false;
}

bool
VDBUnmerger::check_misc(const std::shared_ptr<const ContentsEntry> & e) const
{
    const FSEntry f(e->location_key()->value());

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    return false;
}

bool
VDBUnmerger::check_dir(const std::shared_ptr<const ContentsEntry> & e) const
{
    const FSEntry f(e->location_key()->value());

    if (! (_imp->options.root() / f).exists())
        display("--- [gone ] " + stringify(f));
    else if (! (_imp->options.root() / f).is_directory())
        display("--- [!type] " + stringify(f));
    else if (DirIterator(_imp->options.root() / f, { dio_include_dotfiles, dio_first_only }) != DirIterator())
        display("--- [!empt] " + stringify(f));
    else
        return true;

    return false;
}

void
VDBUnmerger::display(const std::string & message) const
{
    _imp->options.output_manager()->stdout_stream() << message << std::endl;
}

VDBUnmergerError::VDBUnmergerError(const std::string & s) throw () :
    UnmergerError(s)
{
}

