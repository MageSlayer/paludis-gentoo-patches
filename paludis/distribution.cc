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

#include <paludis/distribution.hh>
#include <paludis/hashed_containers.hh>
#include <paludis/config_file.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/system.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include "config.h"

using namespace paludis;

template class InstantiationPolicy<DistributionData, instantiation_method::SingletonTag>;

#include <paludis/distribution-sr.cc>

DistributionConfigurationError::DistributionConfigurationError(const std::string & s) throw () :
    ConfigurationError("Distribution configuration error: " + s)
{
}

namespace paludis
{
    template <>
    struct Implementation<DistributionData>
    {
        MakeHashedMap<std::string, tr1::shared_ptr<const Distribution> >::Type values;
        mutable tr1::shared_ptr<const Distribution> default_distribution;

        Implementation()
        {
            Context c("When loading distribution data:");

            for (DirIterator d(getenv_with_default("PALUDIS_DISTRIBUTIONS_DIR", DATADIR "/paludis/distributions")), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                    continue;

                Context cc("When loading distribution file '" + stringify(*d) + "':");

                KeyValueConfigFile k(*d, KeyValueConfigFileOptions());

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"),
                            make_shared_ptr(new Distribution(Distribution::create()
                                    .default_environment(k.get("default_environment"))
                                    .fallback_environment(k.get("fallback_environment"))
                                    .support_old_style_virtuals(destringify<bool>(k.get("support_old_style_virtuals")))
                                    .default_ebuild_distdir(k.get("default_ebuild_distdir"))
                                    .default_ebuild_write_cache(k.get("default_ebuild_write_cache"))
                                    .default_ebuild_names_cache(k.get("default_ebuild_names_cache"))
                                    .default_ebuild_build_root(k.get("default_ebuild_build_root"))
                                    .default_ebuild_layout(k.get("default_ebuild_layout"))
                                    .eapi_when_unknown(k.get("eapi_when_unknown"))
                                    .eapi_when_unspecified(k.get("eapi_when_unspecified"))
                                    ))));
            }
        }
    };
}

DistributionData::DistributionData() :
    PrivateImplementationPattern<DistributionData>(new Implementation<DistributionData>)
{
}

DistributionData::~DistributionData()
{
}

tr1::shared_ptr<const Distribution>
DistributionData::distribution_from_string(const std::string & s) const
{
    MakeHashedMap<std::string, tr1::shared_ptr<const Distribution> >::Type::const_iterator i(_imp->values.find(s));
    if (i == _imp->values.end())
        return tr1::shared_ptr<const Distribution>();
    else
        return i->second;
}

tr1::shared_ptr<const Distribution>
DistributionData::default_distribution() const
{
    if (_imp->default_distribution)
        return _imp->default_distribution;

    _imp->default_distribution = distribution_from_string(getenv_with_default("PALUDIS_DISTRIBUTION", DEFAULT_DISTRIBUTION));
    if (! _imp->default_distribution)
        throw DistributionConfigurationError("No default distribution configuration available");
    return _imp->default_distribution;
}

