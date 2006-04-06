/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

using namespace paludis;

namespace paludis
{
    class Syncer :
        private InstantiationPolicy<Syncer, instantiation_method::NonCopyableTag>,
        public InternalCounted<Syncer>
    {
        protected:
            Syncer()
            {
            }

        public:
            virtual ~Syncer()
            {
            }

            virtual void sync() const = 0;
    };

    /**
     * Thrown if a sync fails.
     *
     * \ingroup Exception
     */
    class SyncFailedError :
        public PackageActionError
    {
        protected:
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
     * \ingroup Exception
     */
    class NoSuchSyncerError : public SyncFailedError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchSyncerError(const std::string & format) throw ();
    };


    typedef VirtualConstructor<std::string, Syncer::Pointer (*) (const std::string &, const std::string &),
            virtual_constructor_not_found::ThrowException<NoSuchSyncerError> > SyncerMaker;
}

#endif
