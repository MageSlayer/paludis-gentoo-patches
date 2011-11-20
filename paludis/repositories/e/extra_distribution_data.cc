/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/repositories/e/extra_distribution_data.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/upper_lower.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/distribution-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::shared_ptr<const Set<std::string> > make_set(const std::string & s)
    {
        std::shared_ptr<Set<std::string> > result(std::make_shared<Set<std::string>>());
        tokenise_whitespace(s, result->inserter());
        return result;
    }
}

namespace paludis
{
    template <>
    struct ExtraDistributionDataData<EDistribution>
    {
        static std::string config_file_name()
        {
            return "e.conf";
        }

        static std::shared_ptr<EDistribution> make_data(const std::shared_ptr<const KeyValueConfigFile> & k)
        {
            return std::make_shared<EDistribution>(make_named_values<EDistribution>(
                            n::default_buildroot() = k->get("default_buildroot"),
                            n::default_distdir() = k->get("default_distdir"),
                            n::default_eapi_when_unknown() = k->get("default_eapi_when_unknown"),
                            n::default_eapi_when_unspecified() = k->get("default_eapi_when_unspecified"),
                            n::default_layout() = k->get("default_layout"),
                            n::default_manifest_hashes() = make_set(toupper(k->get("default_manifest_hashes"))),
                            n::default_names_cache() = k->get("default_names_cache"),
                            n::default_profile_eapi() = k->get("default_profile_eapi"),
                            n::default_profile_layout() = k->get("default_profile_layout"),
                            n::default_thin_manifests() = destringify<bool>(k->get("default_thin_manifests")),
                            n::default_write_cache() = k->get("default_write_cache"),
                            n::news_directory() = FSPath(k->get("news_directory"))
                            ));
        }
    };
}

namespace paludis
{
    template class ExtraDistributionData<EDistribution>;
    template class Singleton<ExtraDistributionData<EDistribution>>;
}
