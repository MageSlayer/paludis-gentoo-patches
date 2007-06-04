/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_EAPI_HH
#define PALUDIS_GUARD_PALUDIS_EAPI_HH 1

#include <paludis/eapi-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/name.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/portage_dep_parser-fwd.hh>

namespace paludis
{
#include <paludis/eapi-sr.hh>

    /**
     * Thrown if an EAPI configuration is broken.
     *
     * \see EAPI
     * \ingroup grpeapi
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE EAPIConfigurationError :
        public ConfigurationError
    {
        public:
            EAPIConfigurationError(const std::string &) throw ();
    };

    /**
     * Holds information on recognised EAPIs.
     *
     * \see EAPI
     * \ingroup grpeapi
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE EAPIData :
        private PrivateImplementationPattern<EAPIData>,
        public InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<EAPIData, instantiation_method::SingletonTag>;

        private:
            EAPIData();
            ~EAPIData();

        public:
            /**
             * Make an EAPI.
             */
            tr1::shared_ptr<const EAPI> eapi_from_string(const std::string &) const;

            /**
             * Make the unknown EAPI.
             */
            tr1::shared_ptr<const EAPI> unknown_eapi() const;
    };
}

#endif
