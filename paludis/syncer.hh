/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/util/kc-fwd.hh>
#include <paludis/util/keys.hh>
#include <paludis/repository.hh>
#include <string>

/** \file
 * Declarations for the Syncer class, which can be used by repositories to
 * simplify syncing.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Options used by Syncer.
     *
     * \see Syncer
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::options, std::string>,
        kc::Field<k::filter_file, FSEntry>,
        kc::Field<k::output_prefix, std::string>
        > SyncOptions;

    /**
     * Parameters for a Syncer.
     *
     * \see Syncer
     * \ingroup g_repository
     * \nosubgrouping
     */
    typedef kc::KeyedClass<
        kc::Field<k::local, std::string>,
        kc::Field<k::remote, std::string>,
        kc::Field<k::environment, const Environment *>
            > SyncerParams;

    /**
     * A Syncer subclass handles syncing Repository instances.
     *
     * \ingroup g_repository
     */
    class PALUDIS_VISIBLE Syncer :
        private InstantiationPolicy<Syncer, instantiation_method::NonCopyableTag>
    {
        protected:
            /**
             * Constructor.
             */
            Syncer();

        public:
            /**
             * Destructor.
             */
            virtual ~Syncer();

            /**
             * Perform the sync.
             */
            virtual void sync(const SyncOptions &) const = 0;
    };

    /**
     * A Syncer subclass that uses a program from the syncers/ directory.
     *
     * \ingroup g_repository
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
            virtual ~DefaultSyncer();

            /**
             * Perform the sync.
             */
            virtual void sync(const SyncOptions &) const;
    };

    /**
     * Thrown if a sync fails.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
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
     * \ingroup g_repository
     * \ingroup g_exceptions
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
