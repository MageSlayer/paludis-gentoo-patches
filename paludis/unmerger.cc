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

#include "unmerger.hh"
#include <paludis/environment.hh>
#include <paludis/hook.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

using namespace paludis;

namespace paludis
{
    template<>
    struct Implementation<Unmerger>
    {
        UnmergerOptions options;

        std::multimap<std::string, std::pair<EntryType, std::tr1::shared_ptr<Unmerger::ExtraInfo> > > unmerge_entries;

        Implementation(const UnmergerOptions & o) :
            options(o)
        {
        }
    };

    typedef std::multimap<std::string, std::pair<EntryType, std::tr1::shared_ptr<Unmerger::ExtraInfo> > >::reverse_iterator UnmergeEntriesIterator;
}

UnmergerError::UnmergerError(const std::string & s) throw () :
    Exception(s)
{
}

Unmerger::ExtraInfo::~ExtraInfo()
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
Unmerger::add_unmerge_entry(const std::string & f, EntryType et, std::tr1::shared_ptr<ExtraInfo> ei)
{
    _imp->unmerge_entries.insert(std::make_pair(f, std::make_pair(et, ei)));
}

void
Unmerger::unmerge()
{
    populate_unmerge_set();

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                              Hook("unmerger_unlink_pre")
                              ("UNLINK_TARGET", stringify(_imp->options[k::root()])))).max_exit_status)
        throw UnmergerError("Unmerge from '" + stringify(_imp->options[k::root()]) + "' aborted by hook");

    for (UnmergeEntriesIterator  i(_imp->unmerge_entries.rbegin()), i_end(_imp->unmerge_entries.rend()) ; i != i_end ; ++i)
    {
        FSEntry f(i->first);
        switch (i->second.first)
        {
            case et_dir:
                unmerge_dir(f, i->second.second);
                continue;

            case et_file:
                unmerge_file(f, i->second.second);
                continue;

            case et_sym:
                unmerge_sym(f, i->second.second);
                continue;

            case et_misc:
                unmerge_misc(f, i->second.second);
                continue;

            case et_nothing:
            case last_et:
                ;
        }

        throw InternalError(PALUDIS_HERE, "Unexpected entry_type '" + stringify((*i).second.first) + "'");
    }

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                              Hook("unmerger_unlink_post")
                              ("UNLINK_TARGET", stringify(_imp->options[k::root()])))).max_exit_status)
        throw UnmergerError("Unmerge from '" + stringify(_imp->options[k::root()]) + "' aborted by hook");
}

void
Unmerger::unmerge_file(FSEntry & f, std::tr1::shared_ptr<ExtraInfo> ei) const
{
    FSEntry f_real(_imp->options[k::root()] / f);

    HookResult hr(_imp->options[k::environment()]->perform_hook(extend_hook(
                    Hook("unmerger_unlink_file_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status != 0)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
    else if (hr.output == "skip")
        display("--- [skip ] " + stringify(f));
    else if (hr.output == "force")
    {
        display("<<< [force] " + stringify(f));
        unlink_file(f_real, ei);
    }
    else if (check_file(f, ei))
    {
        display("<<<         " + stringify(f));
        unlink_file(f_real, ei);
    }
}

void
Unmerger::unmerge_sym(FSEntry & f, std::tr1::shared_ptr<ExtraInfo> ei) const
{
    FSEntry f_real(_imp->options[k::root()] / f);

    HookResult hr(_imp->options[k::environment()]->perform_hook(extend_hook(
                    Hook("unmerger_unlink_sym_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status != 0)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
    else if (hr.output == "skip")
        display("--- [skip ] " + stringify(f));
    else if (hr.output == "force")
    {
        display("<<< [force] " + stringify(f));
        unlink_sym(f_real, ei);
    }
    else if (check_sym(f, ei))
    {
        display("<<<         " + stringify(f));
        unlink_sym(f_real, ei);
    }
}

void
Unmerger::unmerge_dir(FSEntry & f, std::tr1::shared_ptr<ExtraInfo> ei) const
{
    FSEntry f_real(_imp->options[k::root()] / f);

    HookResult hr(_imp->options[k::environment()]->perform_hook(extend_hook(
                    Hook("unmerger_unlink_dir_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")))));

    if (hr.max_exit_status != 0)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
    else if (hr.output == "skip")
        display("--- [skip ] " + stringify(f));
    else if (check_dir(f, ei))
    {
        display("<<<         " + stringify(f));
        unlink_dir(f_real, ei);
    }
}

void
Unmerger::unmerge_misc(FSEntry & f, std::tr1::shared_ptr<ExtraInfo> ei) const
{
    FSEntry f_real(_imp->options[k::root()] / f);

    HookResult hr(_imp->options[k::environment()]->perform_hook(extend_hook(
                    Hook("unmerger_unlink_misc_override")
                    ("UNLINK_TARGET", stringify(f_real))
                    .grab_output(Hook::AllowedOutputValues()("skip")("force")))));

    if (hr.max_exit_status != 0)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
    else if (hr.output == "skip")
        display("--- [skip ] " + stringify(f));
    else if (hr.output == "force")
    {
        display("<<< [force] " + stringify(f));
        unlink_misc(f_real, ei);
    }
    else if (check_misc(f, ei))
    {
        display("<<<         " + stringify(f));
        unlink_misc(f_real, ei);
    }
}

void
Unmerger::unlink_file(FSEntry & f, std::tr1::shared_ptr<ExtraInfo>) const
{
    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_file_pre")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");

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

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_file_post")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
}

void
Unmerger::unlink_sym(FSEntry & f, std::tr1::shared_ptr<ExtraInfo>) const
{
    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_pre")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");

    f.unlink();

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_sym_post")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
}

void
Unmerger::unlink_dir(FSEntry & f, std::tr1::shared_ptr<ExtraInfo>) const
{
    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_pre")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");

    f.rmdir();

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_dir_post")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
}

void
Unmerger::unlink_misc(FSEntry & f, std::tr1::shared_ptr<ExtraInfo>) const
{
    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_pre")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");

    f.unlink();

    if (0 != _imp->options[k::environment()]->perform_hook(extend_hook(
                         Hook("unmerger_unlink_misc_post")
                         ("UNLINK_TARGET", stringify(f)))).max_exit_status)
        throw UnmergerError("Unmerge of '" + stringify(f) + "' aborted by hook");
}

Hook
Unmerger::extend_hook(const Hook & h) const
{
    return h
        ("ROOT", stringify(_imp->options[k::root()]));
}

