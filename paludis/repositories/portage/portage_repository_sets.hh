/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_SETS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_PORTAGE_PORTAGE_REPOSITORY_SETS_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/repository.hh>

/** \file
 * Declaration for the PortageRepositorySets class.
 *
 * \ingroup grpportagerepository
 */

namespace paludis
{
    class Environment;
    class PortageRepository;

    /**
     * Holds the information about sets, except system, for a PortageRepository.
     *
     * \ingroup grpportagerepository
     */
    class PortageRepositorySets :
        private PrivateImplementationPattern<PortageRepositorySets>,
        private InstantiationPolicy<PortageRepositorySets, instantiation_method::NonCopyableTag>,
        public InternalCounted<PortageRepositorySets>
    {
        private:
            PackageDatabaseEntryCollection::Iterator
            find_best(PackageDatabaseEntryCollection & c, const PackageDatabaseEntry & e) const;

        public:
            ///\name Basic operations
            ///\{

            PortageRepositorySets(const Environment * const env, const PortageRepository * const);
            ~PortageRepositorySets();

            ///\}

            /**
             * Fetch a package set other than system.
             */
            DepAtom::Pointer package_set(const std::string & s, const PackageSetOptions & o) const;

            /**
             * Fetch the security set.
             */
            DepAtom::Pointer security_set(const PackageSetOptions & o) const;
    };
}


#endif
