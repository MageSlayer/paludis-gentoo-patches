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
#include <paludis/util/system.hh>

NoSuchSyncerError::NoSuchSyncerError(const std::string & format) throw () :
    SyncFailedError("No such syncer for format '" + format + "'")
{
}

namespace
{
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
            virtual void sync() const;

            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new RsyncSyncer(local, remote));
            }
    };

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
            }

        public:
            virtual void sync() const;
            static Syncer::Pointer make(const std::string & local, const std::string & remote)
            {
                return Syncer::Pointer(new SvnSyncer(local, remote));
            }
    };

    static const SyncerMaker::RegisterMaker register_rsync_syncer("rsync",  &RsyncSyncer::make);
    static const SyncerMaker::RegisterMaker register_svn_syncer("svn", &SvnSyncer::make);
    static const SyncerMaker::RegisterMaker register_svnplusssh_syncer("svn+ssh", &SvnSyncer::make);
}

void
RsyncSyncer::sync() const
{
    Context context("When performing sync via rsync from '" + _remote + "' to '"
            + _local + "':");

    std::string cmd("rsync --recursive --links --safe-links --perms --times "
            "--compress --force --whole-file --delete --delete-after --stats "
            "--timeout=180 --exclude=/distfiles --exclude=/packages --progress "
            "'" + _remote + "' '" + _local + "/'");

    if (0 != run_command(cmd))
        throw SyncFailedError(_local, _remote);
}

void
SvnSyncer::sync() const
{
    Context context("When performing sync via subversion from '" + _remote + "' to '"
            + _local + "':");

    std::string cmd("svn checkout '" + _remote + "' '" + _local + "/'");

    if (0 != run_command(cmd))
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

