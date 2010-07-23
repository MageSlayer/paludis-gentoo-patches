/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010 Ciaran McCreesh
 * Copyright (c) 2006 Danny van Dyk
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_SETS_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_REPOSITORY_SETS_HH 1

#include <paludis/dep_spec-fwd.hh>
#include <paludis/repository-fwd.hh>

/** \file
 * Declaration for the ERepositorySets class.
 *
 * \ingroup grperepository
 */

namespace paludis
{
    class Environment;
    class ERepository;

    /**
     * Holds the information about sets, except system, for a ERepository.
     *
     * \ingroup grperepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE ERepositorySets :
        private Pimp<ERepositorySets>
    {
        public:
            ///\name Basic operations
            ///\{

            ERepositorySets(const Environment * const env, const ERepository * const,
                    const erepository::ERepositoryParams &);
            ~ERepositorySets();

            ERepositorySets(const ERepositorySets &) = delete;
            ERepositorySets & operator= (const ERepositorySets &) = delete;

            ///\}

            /**
             * Fetch a package set other than system.
             */
            const std::shared_ptr<const SetSpecTree> package_set(const SetName & s) const;

            /**
             * Fetch the security or insecurity set.
             */
            const std::shared_ptr<const SetSpecTree> security_set(bool insecure) const;

            /**
             * Give a list of all the sets in this repo.
             */
            std::shared_ptr<const SetNameSet> sets_list() const;
    };
}


#endif
