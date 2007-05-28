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

#ifndef PALUDIS_GUARD_PALUDIS_DISTRIBUTION_HH
#define PALUDIS_GUARD_PALUDIS_DISTRIBUTION_HH 1

#include <paludis/distribution-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/instantiation_policy.hh>

namespace paludis
{
#include <paludis/distribution-sr.hh>

    class PALUDIS_VISIBLE DistributionConfigurationError :
        public ConfigurationError
    {
        public:
            DistributionConfigurationError(const std::string &) throw ();
    };

    class PALUDIS_VISIBLE DistributionData :
        private PrivateImplementationPattern<DistributionData>,
        public InstantiationPolicy<DistributionData, instantiation_method::SingletonTag>
    {
        friend class InstantiationPolicy<DistributionData, instantiation_method::SingletonTag>;

        private:
            DistributionData();
            ~DistributionData();

        public:
            tr1::shared_ptr<const Distribution> distribution_from_string(const std::string &) const;

            tr1::shared_ptr<const Distribution> default_distribution() const;
    };
}

#endif
