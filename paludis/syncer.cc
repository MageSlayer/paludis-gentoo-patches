/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/syncer.hh>
#include <paludis/environment.hh>

#include <paludis/util/fs_stat.hh>
#include <paludis/util/log.hh>
#include <paludis/util/system.hh>
#include <paludis/util/process.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/env_var_names.hh>
#include <paludis/util/sequence.hh>

#include <paludis/output_manager.hh>

#include <list>

using namespace paludis;

NoSuchSyncerError::NoSuchSyncerError(const std::string & format) noexcept :
    SyncFailedError("No such syncer for format '" + format + "'")
{
}

SyncFailedError::SyncFailedError(const std::string & local, const std::string & remote) noexcept :
    Exception("sync of '" + local + "' from '" + remote + "' failed")
{
}

SyncFailedError::SyncFailedError(const std::string & msg) noexcept :
    Exception(msg)
{
}

DefaultSyncer::DefaultSyncer(const SyncerParams & params) :
    _local(params.local()),
    _remote(params.remote()),
    _revision(params.revision()),
    _environment(params.environment())
{
    std::string::size_type p(_remote.find("://"));
    std::string::size_type q(_remote.find(':'));
    if (std::string::npos == p)
        throw NoSuchSyncerError(_remote);

    const std::string & format = _remote.substr(0, std::min(p, q));
    if (q < p)
        _remote = _remote.substr(q < p ? q + 1 : 0);

    Log::get_instance()->message("syncer.protocol", ll_debug, lc_context) << "looking for syncer protocol '"
        + stringify(format) << "'";

    std::shared_ptr<const FSPathSequence> syncer_dirs(_environment->syncers_dirs());
    FSPath syncer("/var/empty");
    bool ok(false);
    for (const auto & dir : *syncer_dirs)
    {
        syncer = dir / ("do" + format);
        FSStat syncer_stat(syncer);
        if (syncer_stat.exists() && 0 != (syncer_stat.permissions() & S_IXUSR))
            ok = true;

        Log::get_instance()->message("syncer.trying_file", ll_debug, lc_no_context)
            << "Trying '" << syncer << "': " << (ok ? "ok" : "not ok");

        if (ok)
            break;
    }

    if (! ok)
        throw NoSuchSyncerError(format);

    _syncer = stringify(syncer);
}

void
DefaultSyncer::sync(const SyncOptions & opts) const
{
    std::shared_ptr<const FSPathSequence> bashrc_files(_environment->bashrc_files());
    std::shared_ptr<const FSPathSequence> fetchers_dirs(_environment->fetchers_dirs());
    std::shared_ptr<const FSPathSequence> syncers_dirs(_environment->syncers_dirs());

    std::string revision;
    if (! _revision.empty())
        revision = " --revision='" + _revision + "'";

    Process process(ProcessCommand(stringify(_syncer) + " " + opts.options() + revision + " '" + _local + "' '" + _remote + "'"));

    process
        .setenv("PALUDIS_ACTION", "sync")
        .setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .setenv("PALUDIS_FETCHERS_DIRS", join(fetchers_dirs->begin(), fetchers_dirs->end(), " "))
        .setenv("PALUDIS_SYNCERS_DIRS", join(syncers_dirs->begin(), syncers_dirs->end(), " "))
        .setenv("PALUDIS_EBUILD_DIR", getenv_with_default(env_vars::ebuild_dir, LIBEXECDIR "/paludis"))
        .setenv("PALUDIS_SYNC_FILTER_FILE", stringify(opts.filter_file()))
        .capture_stderr(opts.output_manager()->stderr_stream())
        .capture_stdout(opts.output_manager()->stdout_stream())
        .use_ptys();

    if (0 != process.run().wait())
        throw SyncFailedError(_local, _remote);
}
