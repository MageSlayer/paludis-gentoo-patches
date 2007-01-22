/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 * Copyright (c) 2006 Stephen Klimaszewski <steev@gentoo.org>
 * Copyright (c) 2007 David Leverton <u01drl3@abdn.ac.uk>
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

#include "syncer.hh"
#include <paludis/about.hh>
#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <list>

/** \file
 * Implementation for Syncer classes.
 *
 * \ingroup grpsyncer
 */

using namespace paludis;

#include <paludis/syncer-sr.cc>

NoSuchSyncerError::NoSuchSyncerError(const std::string & format) throw () :
    SyncFailedError("No such syncer for format '" + format + "'")
{
}

SyncFailedError::SyncFailedError(const std::string & local, const std::string & remote) throw () :
    PackageActionError("sync of '" + local + "' from '" + remote + "' failed")
{
}

SyncFailedError::SyncFailedError(const std::string & msg) throw () :
    PackageActionError(msg)
{
}

DefaultSyncer::DefaultSyncer(const SyncerParams & params)
    : _local(params.local), _remote(params.remote), _environment(params.environment)
{
    std::string::size_type p(_remote.find("://")), q(_remote.find(":"));
    if (std::string::npos == p)
        throw NoSuchSyncerError(_remote);

    const std::string & format = _remote.substr(0, std::min(p, q));
    if (q < p)
        _remote = _remote.substr(q < p ? q + 1 : 0);

    std::list<std::string> syncers_dirs;
    WhitespaceTokeniser::get_instance()->tokenise(_environment->syncers_dirs(),
            std::back_inserter(syncers_dirs));

    Log::get_instance()->message(ll_debug, lc_context, "looking for syncer protocol '"
            + stringify(format) + "'");

    FSEntry syncer("/var/empty");
    bool ok(false);
    for (std::list<std::string>::const_iterator d(syncers_dirs.begin()),
            d_end(syncers_dirs.end()) ; d != d_end && ! ok; ++d)
    {
        syncer = FSEntry(*d) / ("do" + format);
        if (syncer.exists() && syncer.has_permission(fs_ug_owner, fs_perm_execute))
            ok = true;

        Log::get_instance()->message(ll_debug, lc_no_context, "Trying '" + stringify(syncer) + "': "
                + (ok ? "ok" : "not ok"));
    }

    if (! ok)
        throw NoSuchSyncerError(format);

    _syncer = stringify(syncer);
}

void
DefaultSyncer::sync(const SyncOptions & opts) const
{
    std::list<std::string> remote_list;
    WhitespaceTokeniser::get_instance()->tokenise(_remote, std::back_inserter(remote_list));

    bool ok(false);
    for (std::list<std::string>::const_iterator r(remote_list.begin()),
            r_end(remote_list.end()) ; r != r_end ; ++r)
    {
        MakeEnvCommand cmd(make_env_command(stringify(_syncer) + " '" + _local + "' '" + *r + "'")
                ("PKGMANAGER", PALUDIS_PACKAGE "-" + stringify(PALUDIS_VERSION_MAJOR) + "." +
                         stringify(PALUDIS_VERSION_MINOR) + "." +
                         stringify(PALUDIS_VERSION_MICRO) +
                         (std::string(PALUDIS_SUBVERSION_REVISION).empty() ?
                          std::string("") : "-r" + std::string(PALUDIS_SUBVERSION_REVISION)))
                ("PALUDIS_CONFIG_DIR", SYSCONFDIR "/paludis/")
                ("PALUDIS_BASHRC_FILES", _environment->bashrc_files())
                ("PALUDIS_HOOK_DIRS", _environment->hook_dirs())
                ("PALUDIS_FETCHERS_DIRS", _environment->fetchers_dirs())
                ("PALUDIS_SYNCERS_DIRS", _environment->syncers_dirs())
                ("PALUDIS_COMMAND", _environment->paludis_command())
                ("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
                ("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
                ("PALUDIS_SYNC_EXCLUDE_FROM", opts.exclude_from));
        if (! run_command(cmd))
        {
            ok = true;
            break;
        }
    }

    if (! ok)
        throw SyncFailedError(_local, _remote);
}

