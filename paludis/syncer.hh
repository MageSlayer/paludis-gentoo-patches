/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/output_manager-fwd.hh>
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
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_filter_file> filter_file;
        typedef Name<struct name_local> local;
        typedef Name<struct name_options> options;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_remote> remote;
        typedef Name<struct name_revision> revision;
    }

    /**
     * Options used by Syncer.
     *
     * \ingroup g_repository
     * \nosubgrouping
     */
    struct SyncOptions
    {
        NamedValue<n::filter_file, FSPath> filter_file;
        NamedValue<n::options, std::string> options;

        /**
         * \since 0.36
         */
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
    };

    /**
     * Parameters for a Syncer.
     *
     * \see Syncer
     * \ingroup g_repository
     * \nosubgrouping
     */
    struct SyncerParams
    {
        NamedValue<n::environment, const Environment *> environment;

        NamedValue<n::local, std::string> local;

        NamedValue<n::remote, std::string> remote;

        /**
         * Usually empty, and not supported by most syncers.
         *
         * \since 0.61
         */
        NamedValue<n::revision, std::string> revision;
    };

    /**
     * A Syncer subclass handles syncing Repository instances.
     *
     * \ingroup g_repository
     */
    class PALUDIS_VISIBLE Syncer
    {
        protected:
            /**
             * Constructor.
             */
            Syncer() = default;

        public:
            /**
             * Destructor.
             */
            virtual ~Syncer() = default;

            Syncer(const Syncer &) = delete;
            Syncer & operator= (const Syncer &) = delete;

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
            std::string _local, _remote, _revision;
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
            ~DefaultSyncer() override = default;

            /**
             * Perform the sync.
             */
            void sync(const SyncOptions &) const override;
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
            SyncFailedError(const std::string & msg) noexcept;

            /**
             * Constructor.
             */
            SyncFailedError(const std::string & local, const std::string & remote) noexcept;
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
            NoSuchSyncerError(const std::string & format) noexcept;
    };
}

#endif
