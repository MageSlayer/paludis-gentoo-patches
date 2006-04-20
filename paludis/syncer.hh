/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/util/virtual_constructor.hh>
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
    /**
     * Keys for SyncOptions.
     *
     * \see SyncOptions
     * \ingroup grpsyncer
     */
    enum SyncOptionsKeys
    {
        so_excludefrom,     ///< Filename from which to take a list of excluded files
        so_last             ///< Number of keys.
    };

    /**
     * Tag for SyncOptions.
     *
     * \see SyncOptions
     * \ingroup grpsyncer
     */
    struct SyncOptionsTag :
        SmartRecordTag<comparison_mode::NoComparisonTag, void>,
        SmartRecordKeys<SyncOptionsKeys, so_last>,
        SmartRecordKey<so_excludefrom, std::string>
    {
    };

    /**
     * Defines options to be passed to the syncer.
     *
     * \ingroup grpsyncer
     */
    typedef MakeSmartRecord<SyncOptionsTag>::Type SyncOptions;

    /**
     * A Syncer subclass handles syncing Repository instances.
     *
     * \ingroup grpsyncer
     */
    class Syncer :
        private InstantiationPolicy<Syncer, instantiation_method::NonCopyableTag>,
        public InternalCounted<Syncer>
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
     * Thrown if a sync fails.
     *
     * \ingroup grpsyncer
     * \ingroup grpexceptions
     */
    class SyncFailedError :
        public PackageActionError
    {
        protected:
            /**
             * Constructor.
             */
            SyncFailedError(const std::string & msg) throw ();

        public:
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
    class NoSuchSyncerError : public SyncFailedError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchSyncerError(const std::string & format) throw ();
    };


    /**
     * Virtual constructor for Syncer subclasses.
     *
     * \ingroup grpsyncer
     */
    typedef VirtualConstructor<std::string, Syncer::Pointer (*) (const std::string &, const std::string &),
            virtual_constructor_not_found::ThrowException<NoSuchSyncerError> > SyncerMaker;
}

#endif
