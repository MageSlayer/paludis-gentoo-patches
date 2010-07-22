/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
 * Copyright (c) 2006 Stephen Klimaszewski
 * Copyright (c) 2007 David Leverton
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
#include <paludis/environment.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/sequence.hh>
#include <paludis/output_manager.hh>
#include <list>

using namespace paludis;

NoSuchSyncerError::NoSuchSyncerError(const std::string & format) throw () :
    SyncFailedError("No such syncer for format '" + format + "'")
{
}

SyncFailedError::SyncFailedError(const std::string & local, const std::string & remote) throw () :
    Exception("sync of '" + local + "' from '" + remote + "' failed")
{
}

SyncFailedError::SyncFailedError(const std::string & msg) throw () :
    Exception(msg)
{
}

DefaultSyncer::DefaultSyncer(const SyncerParams & params) :
    _local(params.local()),
    _remote(params.remote()),
    _environment(params.environment())
{
    std::string::size_type p(_remote.find("://")), q(_remote.find(":"));
    if (std::string::npos == p)
        throw NoSuchSyncerError(_remote);

    const std::string & format = _remote.substr(0, std::min(p, q));
    if (q < p)
        _remote = _remote.substr(q < p ? q + 1 : 0);

    Log::get_instance()->message("syncer.protocol", ll_debug, lc_context) << "looking for syncer protocol '"
        + stringify(format) << "'";

    std::shared_ptr<const FSEntrySequence> syncer_dirs(_environment->syncers_dirs());
    FSEntry syncer("/var/empty");
    bool ok(false);
    for (FSEntrySequence::ConstIterator d(syncer_dirs->begin()), d_end(syncer_dirs->end()) ;
            d != d_end && ! ok; ++d)
    {
        syncer = FSEntry(*d) / ("do" + format);
        if (syncer.exists() && syncer.has_permission(fs_ug_owner, fs_perm_execute))
            ok = true;

        Log::get_instance()->message("syncer.trying_file", ll_debug, lc_no_context)
            << "Trying '" << syncer << "': " << (ok ? "ok" : "not ok");
    }

    if (! ok)
        throw NoSuchSyncerError(format);

    _syncer = stringify(syncer);
}

void
DefaultSyncer::sync(const SyncOptions & opts) const
{
    std::shared_ptr<const FSEntrySequence> bashrc_files(_environment->bashrc_files());
    std::shared_ptr<const FSEntrySequence> fetchers_dirs(_environment->fetchers_dirs());
    std::shared_ptr<const FSEntrySequence> syncers_dirs(_environment->syncers_dirs());

    Command cmd(Command(stringify(_syncer) + " " + opts.options() + " '" + _local + "' '" + _remote + "'")
            .with_setenv("PALUDIS_ACTION", "sync")
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
            .with_setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
            .with_setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
            .with_setenv("PALUDIS_EBUILD_DIR", getenv_with_default("PALUDIS_EBUILD_DIR", LIBEXECDIR "/paludis"))
            .with_setenv("PALUDIS_SYNC_FILTER_FILE", stringify(opts.filter_file())));

    cmd
        .with_captured_stderr_stream(&opts.output_manager()->stderr_stream())
        .with_captured_stdout_stream(&opts.output_manager()->stdout_stream())
        .with_ptys();

    if (run_command(cmd))
        throw SyncFailedError(_local, _remote);
}

Syncer::Syncer()
{
}

Syncer::~Syncer()
{
}

DefaultSyncer::~DefaultSyncer()
{
}

