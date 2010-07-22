/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/distribution-impl.hh>
#include <paludis/util/config_file.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/hashes.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <unordered_map>

using namespace paludis;

template class Singleton<DistributionData>;

DistributionConfigurationError::DistributionConfigurationError(const std::string & s) throw () :
    ConfigurationError("Distribution configuration error: " + s)
{
}

typedef std::unordered_map<std::string, std::shared_ptr<const Distribution>, Hash<std::string> > DistributionHash;

namespace paludis
{
    template <>
    struct Implementation<DistributionData>
    {
        DistributionHash values;

        Implementation()
        {
            Context c("When loading distribution data:");

            for (DirIterator d(getenv_with_default("PALUDIS_DISTRIBUTIONS_DIR", DATADIR "/paludis/distributions")), d_end ;
                    d != d_end ; ++d)
            {
                if (! is_file_with_extension(*d, ".conf", IsFileWithOptions()))
                    continue;

                Context cc("When loading distribution file '" + stringify(*d) + "':");

                KeyValueConfigFile k(*d, KeyValueConfigFileOptions(), &KeyValueConfigFile::no_defaults, &KeyValueConfigFile::no_transformation);

                values.insert(std::make_pair(strip_trailing_string(d->basename(), ".conf"),
                            std::make_shared<Distribution>(make_named_values<Distribution>(
                                        n::concept_keyword() = k.get("concept_keyword"),
                                        n::concept_license() = k.get("concept_license"),
                                        n::concept_use() = k.get("concept_use"),
                                        n::default_environment() = k.get("default_environment"),
                                        n::extra_data_dir() = FSEntry(strip_trailing_string(stringify(d->realpath()), ".conf")),
                                        n::fallback_environment() = k.get("fallback_environment"),
                                        n::name() = strip_trailing_string(d->basename(), ".conf"),
                                        n::paludis_package() = k.get("paludis_package"),
                                        n::support_old_style_virtuals() = destringify<bool>(k.get("support_old_style_virtuals"))
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

std::shared_ptr<const Distribution>
DistributionData::distribution_from_string(const std::string & s) const
{
    DistributionHash::const_iterator i(_imp->values.find(s));
    if (i == _imp->values.end())
        throw DistributionConfigurationError("No distribution configuration found for '" + s + "'");
    else
        return i->second;
}

