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
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/set.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/output_manager.hh>
#include <paludis/metadata_key.hh>
#include <tr1/functional>
#include <list>
#include <map>
#include <vector>

using namespace paludis;

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
            tokenise_whitespace(o.config_protect(), std::back_inserter(config_protect));
            tokenise_whitespace(o.config_protect_mask(), std::back_inserter(config_protect_mask));
        }
    };
}

NDBAMUnmerger::NDBAMUnmerger(const NDBAMUnmergerOptions & o) :
    Unmerger(make_named_values<UnmergerOptions>(
            value_for<n::environment>(o.environment()),
            value_for<n::ignore>(o.ignore()),
            value_for<n::root>(o.root())
            )),
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
    std::string root_str(stringify(_imp->options.root())), f_str(stringify(f));
    if (root_str == "/")
        root_str.clear();
    if (0 != f_str.compare(0, root_str.length(), root_str))
        throw NDBAMUnmergerError("Can't work out tidy name for '" + f_str + "' with root '" + root_str + "'");
    return f_str.substr(root_str.length());
}

void
NDBAMUnmerger::_add_file(const std::tr1::shared_ptr<const ContentsEntry> & e)
{
    add_unmerge_entry(et_file, e);
}

void
NDBAMUnmerger::_add_dir(const std::tr1::shared_ptr<const ContentsEntry> & e)
{
    add_unmerge_entry(et_dir, e);
}

void
NDBAMUnmerger::_add_sym(const std::tr1::shared_ptr<const ContentsEntry> & e)
{
    add_unmerge_entry(et_sym, e);
}

void
NDBAMUnmerger::populate_unmerge_set()
{
    using namespace std::tr1::placeholders;
    _imp->options.ndbam()->parse_contents(*_imp->options.package_id(),
            std::tr1::bind(&NDBAMUnmerger::_add_file, this, _1),
            std::tr1::bind(&NDBAMUnmerger::_add_dir, this, _1),
            std::tr1::bind(&NDBAMUnmerger::_add_sym, this, _1)
            );
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
NDBAMUnmerger::check_file(const std::tr1::shared_ptr<const ContentsEntry> & e) const
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
        SafeIFStream md5_file(_imp->options.root() / f);
        if (! md5_file)
        {
            Log::get_instance()->message("ndbam.unmerger.md5_failed", ll_warning, lc_no_context) << "Cannot get md5 for '" <<
                (_imp->options.root() / f) << "'";
            display("--- [!md5?] " + stringify(f));
        }
        else if (MD5(md5_file).hexsum() != require_key<MetadataValueKey<std::string> >(*e, "md5").value())
            display("--- [!md5 ] " + stringify(f));
        else if (config_protected(_imp->options.root() / f))
            display("--- [cfgpr] " + stringify(f));
        else
            return true;
    }

    return false;
}

bool
NDBAMUnmerger::check_sym(const std::tr1::shared_ptr<const ContentsEntry> & e) const
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
NDBAMUnmerger::check_misc(const std::tr1::shared_ptr<const ContentsEntry> &) const
{
    return false;
}

bool
NDBAMUnmerger::check_dir(const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    const FSEntry f(e->location_key()->value());

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
NDBAMUnmerger::display(const std::string & message) const
{
    _imp->options.output_manager()->stdout_stream() << message << std::endl;
}

NDBAMUnmergerError::NDBAMUnmergerError(const std::string & s) throw () :
    UnmergerError(s)
{
}

