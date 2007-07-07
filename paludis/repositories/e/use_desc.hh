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

#ifndef PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_USE_DESC_HH
#define PALUDIS_GUARD_PALUDIS_PALUDIS_REPOSITORIES_E_USE_DESC_HH 1

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    class FSEntry;

    /**
     * Implements use.desc options for ERepository.
     *
     * \ingroup grperepository
     */
    class UseDesc :
        private PrivateImplementationPattern<UseDesc>,
        private InstantiationPolicy<UseDesc, instantiation_method::NonCopyableTag>
    {
        public:
            ///\name Basic operations
            ///\{

            UseDesc(const FSEntry &, const std::string & expand_sep);
            ~UseDesc();

            ///\}

            /**
             * Describe the given use flag, possibly for the specified package.
             *
             * May return an empty string.
             */
            std::string describe(const UseFlagName &, const PackageID &) const;
    };
}

#endif
