/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_SYNCER_HH
#define PALUDIS_GUARD_PALUDIS_SYNCER_HH 1

#include <paludis/util/exception.hh>
#include <paludis/repository.hh>
#include <string>

/** \file
 * Declarations for the Syncer classes.
 *
 * \ingroup grpsyncer
 */

namespace paludis
{

#include <paludis/syncer-sr.hh>

    /**
     * A Syncer subclass handles syncing Repository instances.
     *
     * \ingroup grpsyncer
     */
    class PALUDIS_VISIBLE Syncer :
        private InstantiationPolicy<Syncer, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Constructor.
             */
            Syncer()
            {
            }

        public:
            /**
             * Destructor.
             */
            virtual ~Syncer()
            {
            }

            /**
             * Perform the sync.
             */
            virtual void sync(const SyncOptions &) const = 0;
    };

    /**
     * A Syncer subclass that uses a program from the syncers/ directory.
     *
     * \ingroup grpsyncer
     */
    class PALUDIS_VISIBLE DefaultSyncer :
        public Syncer
    {
        private:
            std::string _local, _remote;
            const Environment *_environment;

            std::string _syncer;

        public:
            /**
             * Constructor.
             */
            DefaultSyncer(const SyncerParams &);

            /**
             * Destructor.
             */
            virtual ~DefaultSyncer()
            {
            }

            /**
             * Perform the sync.
             */
            virtual void sync(const SyncOptions &) const;
    };

    /**
     * Thrown if a sync fails.
     *
     * \ingroup grpsyncer
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE SyncFailedError :
        public Exception
    {
        public:
            /**
             * Constructor.
             */
            SyncFailedError(const std::string & msg) throw ();

            /**
             * Constructor.
             */
            SyncFailedError(const std::string & local, const std::string & remote) throw ();
    };

    /**
     * Thrown if a syncer of the specified type does not exist.
     *
     * \ingroup grpsyncer
     * \ingroup grpexceptions
     */
    class PALUDIS_VISIBLE NoSuchSyncerError : public SyncFailedError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchSyncerError(const std::string & format) throw ();
    };
}

#endif
