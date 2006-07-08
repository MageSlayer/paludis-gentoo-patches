/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 Stephen Klimaszewski <steev@gentoo.org>
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/system.hh>

/** \file
 * Implementation for Syncer classes.
 *
 * \ingroup grpsyncer
 */

using namespace paludis;

NoSuchSyncerError::NoSuchSyncerError(const std::string & format) throw () :
    SyncFailedError("No such syncer for format '" + format + "'")
{
}

namespace
{
    /**
     * A Syncer for rsync:// syncing.
     *
     * \ingroup grpsyncer
     */
    class RsyncSyncer :
        public Syncer
    {
        private:
            std::string _local;
            std::string _remote;

        protected:
            RsyncSyncer(const std::string & local, const std::string & remote) :
                _local(local),
                _remote(remote)
            {
            }

        public:
            virtual void sync(const SyncOptions &) const;

            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new RsyncSyncer(local, remote));
            }
    };

    /**
     * A Syncer for svn:// (subversion) syncing.
     *
     * \ingroup grpsyncer
     */
    class SvnSyncer :
        public Syncer
    {
        private:
            std::string _local;
            std::string _remote;

        protected:
            SvnSyncer(const std::string & local, const std::string & remote) :
                _local(local),
                _remote(remote)
            {
                if (0 == _remote.compare(0, 8, "svn+http", 0, 8))
                    _remote = _remote.erase(0, 4);
            }

        public:
            virtual void sync(const SyncOptions &) const;
            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new SvnSyncer(local, remote));
            }
    };

    /**
     * A Syncer for Git syncing.
     *
     * \ingroup grpsyncer
     */
    class GitSyncer :
        public Syncer
    {
        private:
            std::string _local;
            std::string _remote;

        protected:
            GitSyncer(const std::string & local, const std::string & remote) :
                _local(local),
                _remote(remote)
            {
                if (0 == _remote.compare(0, 8, "git+http", 0, 8))
                    _remote = _remote.erase(0, 4);
            }

        public:
            virtual void sync(const SyncOptions &) const;
            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new GitSyncer(local, remote));
            }
    };

    /**
     * Register rsync:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_rsync_syncer("rsync",  &RsyncSyncer::make);

    /**
     * Register svn:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_svn_syncer("svn", &SvnSyncer::make);

    /**
     * Register svn+ssh:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_svnplusssh_syncer("svn+ssh", &SvnSyncer::make);

    /**
     * Register svn+http:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_svnplushttp_syncer("svn+http", &SvnSyncer::make);

    /**
     * Register git:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_git_syncer("git", &GitSyncer::make);

    /**
     * Register git+ssh:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_gitplusssh_syncer("git+ssh", &GitSyncer::make);

    /**
     * Register git+http:// protocol.
     *
     * \ingroup grpsyncer
     */
    static const SyncerMaker::RegisterMaker register_gitplushttp_syncer("git+http", &GitSyncer::make);
}

void
RsyncSyncer::sync(const SyncOptions & opts) const
{
    Context context("When performing sync via rsync from '" + _remote + "' to '"
            + _local + "':");

    FSEntry(_local).mkdir();

    std::string exclude;
    if (! opts.get<so_excludefrom>().empty())
        exclude = "--exclude-from " + std::string(opts.get<so_excludefrom>()) + " ";

    std::string cmd("rsync --recursive --links --safe-links --perms --times "
            "--compress --force --whole-file --delete --delete-after --stats "
            "--timeout=180 --exclude=/distfiles --exclude=/packages --progress "
            + exclude + "'" + _remote + "' '" + _local + "/'");

    if (0 != run_command(cmd))
        throw SyncFailedError(_local, _remote);
}

void
SvnSyncer::sync(const SyncOptions &) const
{
    Context context("When performing sync via subversion from '" + _remote + "' to '"
            + _local + "':");

    std::string cmd("svn checkout '" + _remote + "' '" + _local + "/'");

    if (0 != run_command(cmd))
        throw SyncFailedError(_local, _remote);
}

void
GitSyncer::sync(const SyncOptions &) const
{
    Context context("When performing sync via git from '" + _remote + "' to '"
            + _local + "':");

    FSEntry git_dir(_local + "/.git");
    FSEntry repo_dir(_local);
    int status;

    if (repo_dir.is_directory() && ! git_dir.is_directory())
        throw SyncGitDirectoryExists(_local);

    if (git_dir.is_directory())
        status = run_command_in_directory("git pull", repo_dir);
    else
        status = run_command("git clone '"+ _remote + "' '" + _local + "'");

    if (0 != status)
        throw SyncFailedError(_local, _remote);
}

SyncFailedError::SyncFailedError(const std::string & local, const std::string & remote) throw () :
    PackageActionError("sync of '" + local + "' from '" + remote + "' failed")
{
}

SyncFailedError::SyncFailedError(const std::string & msg) throw () :
    PackageActionError(msg)
{
}

SyncGitDirectoryExists::SyncGitDirectoryExists(const std::string & local) throw () :
    SyncFailedError("'" + local + "' exists but it is not a Git repository")
{
}
