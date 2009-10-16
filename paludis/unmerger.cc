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

#include "unmerger.hh"
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/contents.hh>
#include <paludis/metadata_key.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

using namespace paludis;

namespace paludis
{
    typedef std::multimap<std::string, std::pair<EntryType, std::tr1::shared_ptr<const ContentsEntry> > > UnmergeEntries;
    typedef UnmergeEntries::reverse_iterator UnmergeEntriesIterator;

    template<>
    struct Implementation<Unmerger>
    {
        UnmergerOptions options;

        UnmergeEntries unmerge_entries;

        Implementation(const UnmergerOptions & o) :
            options(o)
        {
        }
    };
}

UnmergerError::UnmergerError(const std::string & s) throw () :
    Exception(s)
{
}

Unmerger::Unmerger(const UnmergerOptions & o) :
    PrivateImplementationPattern<Unmerger>(new Implementation<Unmerger>(o))
{
}

Unmerger::~Unmerger()
{
}

void
Unmerger::add_unmerge_entry(const EntryType et, const std::tr1::shared_ptr<const ContentsEntry> & e)
{
    _imp->unmerge_entries.insert(std::make_pair(stringify(e->location_key()->value()), std::make_pair(et, e)));
}

void
Unmerger::unmerge()
{
    populate_unmerge_set();

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                              Hook("unmerger_unlink_pre")
                              ("UNLINK_TARGET", stringify(_imp->options.root())))).max_exit_status())
        throw UnmergerError("Unmerge from '" + stringify(_imp->options.root()) + "' aborted by hook");

    for (UnmergeEntriesIterator  i(_imp->unmerge_entries.rbegin()), i_end(_imp->unmerge_entries.rend()) ; i != i_end ; ++i)
    {
        FSEntry f(i->first);
        switch (i->second.first)
        {
            case et_dir:
                unmerge_dir(i->second.second);
                continue;

            case et_file:
                unmerge_file(i->second.second);
                continue;

            case et_sym:
                unmerge_sym(i->second.second);
                continue;

            case et_misc:
                unmerge_misc(i->second.second);
                continue;

            case et_nothing:
            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify((*i).second.first) + "'");
    }

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                              Hook("unmerger_unlink_post")
                              ("UNLINK_TARGET", stringify(_imp->options.root())))).max_exit_status())
        throw UnmergerError("Unmerge from '" + stringify(_imp->options.root()) + "' aborted by hook");
}

void
Unmerger::unmerge_file(const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    FSEntry f_real(_imp->options.root() / e->location_key()->value());

    HookResult hr(_imp->options.environment()->perform_hook(extend_hook(
                    Hook("unmerger_unlink_file_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status() != 0)
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
    else if (hr.output() == "skip")
        display("--- [skip ] " + stringify(e->location_key()->value()));
    else if (hr.output() == "force")
    {
        display("<<< [force] " + stringify(e->location_key()->value()));
        unlink_file(f_real, e);
    }
    else if (_imp->options.ignore()(f_real))
        display("--- [ignor] " + stringify(e->location_key()->value()));
    else if (check_file(e))
    {
        display("<<<         " + stringify(e->location_key()->value()));
        unlink_file(f_real, e);
    }
}

void
Unmerger::unmerge_sym(const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    FSEntry f_real(_imp->options.root() / e->location_key()->value());

    HookResult hr(_imp->options.environment()->perform_hook(extend_hook(
                    Hook("unmerger_unlink_sym_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status() != 0)
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
    else if (hr.output() == "skip")
        display("--- [skip ] " + stringify(e->location_key()->value()));
    else if (hr.output() == "force")
    {
        display("<<< [force] " + stringify(e->location_key()->value()));
        unlink_sym(f_real, e);
    }
    else if (_imp->options.ignore()(f_real))
        display("--- [ignor] " + stringify(e->location_key()->value()));
    else if (check_sym(e))
    {
        display("<<<         " + stringify(e->location_key()->value()));
        unlink_sym(f_real, e);
    }
}

void
Unmerger::unmerge_dir(const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    FSEntry f_real(_imp->options.root() / e->location_key()->value());

    HookResult hr(_imp->options.environment()->perform_hook(extend_hook(
                    Hook("unmerger_unlink_dir_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")))));

    if (hr.max_exit_status() != 0)
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
    else if (hr.output() == "skip")
        display("--- [skip ] " + stringify(e->location_key()->value()));
    else if (_imp->options.ignore()(f_real))
        display("--- [ignor] " + stringify(e->location_key()->value()));
    else if (check_dir(e))
    {
        display("<<<         " + stringify(e->location_key()->value()));
        unlink_dir(f_real, e);
    }
}

void
Unmerger::unmerge_misc(const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    FSEntry f_real(_imp->options.root() / e->location_key()->value());

    HookResult hr(_imp->options.environment()->perform_hook(extend_hook(
                    Hook("unmerger_unlink_misc_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status() != 0)
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
    else if (hr.output() == "skip")
        display("--- [skip ] " + stringify(e->location_key()->value()));
    else if (hr.output() == "force")
    {
        display("<<< [force] " + stringify(e->location_key()->value()));
        unlink_misc(f_real, e);
    }
    else if (_imp->options.ignore()(f_real))
        display("--- [ignor] " + stringify(e->location_key()->value()));
    else if (check_misc(e))
    {
        display("<<<         " + stringify(e->location_key()->value()));
        unlink_misc(f_real, e);
    }
}

void
Unmerger::unlink_file(FSEntry f, const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_file_pre")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");

    if (f.is_regular_file())
    {
        mode_t mode(f.permissions());
        if ((mode & S_ISUID) || (mode & S_ISGID))
        {
            mode &= 0400;
            f.chmod(mode);
        }
    }

    f.unlink();

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_file_post")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
}

void
Unmerger::unlink_sym(FSEntry f, const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_pre")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");

    f.unlink();

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_post")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
}

void
Unmerger::unlink_dir(FSEntry f, const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_pre")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");

    f.rmdir();

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_post")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
}

void
Unmerger::unlink_misc(FSEntry f, const std::tr1::shared_ptr<const ContentsEntry> & e) const
{
    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_pre")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");

    f.unlink();

    if (0 != _imp->options.environment()->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_post")
                         ("UNLINK_TARGET", stringify(e->location_key()->value())))).max_exit_status())
        throw UnmergerError("Unmerge of '" + stringify(e->location_key()->value()) + "' aborted by hook");
}

Hook
Unmerger::extend_hook(const Hook & h) const
{
    return h
        ("ROOT", stringify(_imp->options.root()));
}

bool
Unmerger::check_file(const std::tr1::shared_ptr<const ContentsEntry> &) const
{
    return true;
}

bool
Unmerger::check_dir(const std::tr1::shared_ptr<const ContentsEntry> &) const
{
    return true;
}

bool
Unmerger::check_sym(const std::tr1::shared_ptr<const ContentsEntry> &) const
{
    return true;
}

bool
Unmerger::check_misc(const std::tr1::shared_ptr<const ContentsEntry> &) const
{
    return true;
}

