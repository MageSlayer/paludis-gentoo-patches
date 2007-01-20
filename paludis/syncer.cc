/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/syncer-sr.cc>

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
                if ((0 == _remote.compare(0, 4, "file", 0, 4)))
                    _remote.erase(0, 7);
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
                if ((0 == _remote.compare(0, 8, "svn+http", 0, 8)) || (0 == _remote.compare(0, 9, "svn+https", 0, 9)))
                    _remote.erase(0, 4);
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
                    _remote.erase(0, 4);
            }

        public:
            virtual void sync(const SyncOptions &) const;
            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new GitSyncer(local, remote));
            }
    };

    /**
     * A Syncer for cvs+ext://, cvs+pserver:// (CVS) syncing.
     *
     * \ingroup grpsyncer
     */
    class CvsSyncer :
        public Syncer
    {
        private:
            std::string _local;
            std::string _remote;
            std::string _module;

            bool _pserver;

        protected:
            CvsSyncer(const std::string & local, const std::string & remote) :
                _local(local),
                _remote(remote),
                _pserver(false)
            {
                if (0 == _remote.compare(0, 11, "cvs+pserver", 0, 11))
                {
                    _remote = ":pserver:" + _remote.erase(0, 14);
                    _pserver = true;
                }
                else if ((0 == _remote.compare(0, 7, "cvs+ssh", 0, 7)) || (0 == _remote.compare(0, 7, "cvs+ext", 0, 7)))
                {
                    _remote = ":ext:" + _remote.erase(0, 10);
                }

                std::string::size_type pos(_remote.find_last_of(":"));

                if (std::string::npos == pos)
                    throw SyncCvsUrlInvalid(_remote);

                _module = std::string(_remote, pos + 1);
                _remote.erase(pos);
            }

        public:
            virtual void sync(const SyncOptions &) const;
            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new CvsSyncer(local, remote));
            }
    };
}

void
RsyncSyncer::sync(const SyncOptions & opts) const
{
    Context context("When performing sync via rsync from '" + _remote + "' to '"
            + _local + "':");

    FSEntry(_local).mkdir();

    std::string exclude;
    if (! opts.exclude_from.empty())
        exclude = "--exclude-from " + opts.exclude_from + " ";

    std::string cmd("rsync --recursive --links --safe-links --perms --times "
            "--compress --force --whole-file --delete --delete-after --stats "
            "--timeout=180 --exclude=/distfiles --exclude=/packages "
            "--exclude=/local --exclude=/.cache --progress " + exclude + "'" + _remote + 
            "' '" + _local + "/'");

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

void
CvsSyncer::sync(const SyncOptions &) const
{
    Context context("When performing sync via CVS from '" + _remote + "' to '"
            + _local + "':");

    std::string cmd("cvs -d '" + _remote + "' login");
    FSEntry d(FSEntry(_local).dirname());

    if (_pserver)
    {
        if (0 != run_command_in_directory(cmd, d))
            throw SyncFailedError(_local, _remote + ":" + _module);
    }

    cmd = std::string("cvs -d '" + _remote + "' checkout '" + _module + "'");
    if (0 != run_command_in_directory(cmd, d))
        throw SyncFailedError(_local, _remote + ":" + _module);
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

SyncCvsUrlInvalid::SyncCvsUrlInvalid(const std::string & url) throw () :
    SyncFailedError("'" + url + "' is not a valid URL for a CVS repository")
{
}

SyncerMaker::SyncerMaker()
{
    register_maker("rsync",  &RsyncSyncer::make);
    register_maker("file", &RsyncSyncer::make);
    register_maker("svn", &SvnSyncer::make);
    register_maker("svn+ssh", &SvnSyncer::make);
    register_maker("svn+http", &SvnSyncer::make);
    register_maker("svn+https", &SvnSyncer::make);
    register_maker("git", &GitSyncer::make);
    register_maker("git+ssh", &GitSyncer::make);
    register_maker("git+http", &GitSyncer::make);
    register_maker("cvs+ext", &CvsSyncer::make);
    register_maker("cvs+ssh", &CvsSyncer::make);
    register_maker("cvs+pserver", &CvsSyncer::make);
}

