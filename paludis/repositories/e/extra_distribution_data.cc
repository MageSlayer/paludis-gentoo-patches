/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/wrapped_output_iterator.hh>
#include <paludis/distribution-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

namespace
{
    std::tr1::shared_ptr<const Set<std::string> > make_set(const std::string & s)
    {
        std::tr1::shared_ptr<Set<std::string> > result(new Set<std::string>);
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

        static std::tr1::shared_ptr<EDistribution> make_data(const std::tr1::shared_ptr<const KeyValueConfigFile> & k)
        {
            return make_shared_ptr(new EDistribution(make_named_values<EDistribution>(
                            value_for<n::default_buildroot>(k->get("default_buildroot")),
                            value_for<n::default_distdir>(k->get("default_distdir")),
                            value_for<n::default_eapi_when_unknown>(k->get("default_eapi_when_unknown")),
                            value_for<n::default_eapi_when_unspecified>(k->get("default_eapi_when_unspecified")),
                            value_for<n::default_layout>(k->get("default_layout")),
                            value_for<n::default_names_cache>(k->get("default_names_cache")),
                            value_for<n::default_profile_eapi>(k->get("default_profile_eapi")),
                            value_for<n::default_profile_layout>(k->get("default_profile_layout")),
                            value_for<n::default_provides_cache>(k->get("default_provides_cache")),
                            value_for<n::default_write_cache>(k->get("default_write_cache")),
                            value_for<n::news_directory>(FSEntry(k->get("news_directory"))),
                            value_for<n::qa_category_dir_checks>(make_set(k->get("qa_category_dir_checks"))),
                            value_for<n::qa_eclass_file_contents_checks>(make_set(k->get("qa_eclass_file_contents_checks"))),
                            value_for<n::qa_package_dir_checks>(make_set(k->get("qa_package_dir_checks"))),
                            value_for<n::qa_package_id_checks>(make_set(k->get("qa_package_id_checks"))),
                            value_for<n::qa_package_id_file_contents_checks>(make_set(k->get("qa_package_id_file_contents_checks"))),
                            value_for<n::qa_tree_checks>(make_set(k->get("qa_tree_checks")))
                            )));
        }
    };
}

template class ExtraDistributionData<EDistribution>;
template class InstantiationPolicy<ExtraDistributionData<EDistribution>, instantiation_method::SingletonTag>;

