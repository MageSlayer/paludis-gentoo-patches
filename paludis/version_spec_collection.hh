/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VERSION_SPEC_COLLECTION_HH
#define PALUDIS_GUARD_PALUDIS_VERSION_SPEC_COLLECTION_HH 1

#include <paludis/version_spec.hh>
#include <paludis/sorted_collection.hh>

namespace paludis
{
    /**
     * Holds a collection of VersionSpec instances.
     */
    class VersionSpecCollection : public SortedCollection<VersionSpec>
    {
        public:
            /**
             * Constructor.
             */
            VersionSpecCollection();

            /**
             * Destructor.
             */
            virtual ~VersionSpecCollection();
    };
}

#endif
