/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/environments/paludis/extra_distribution_data.hh>
#include <paludis/util/destringify.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/singleton-impl.hh>
#include <paludis/distribution-impl.hh>

using namespace paludis;
using namespace paludis::paludis_environment;

namespace paludis
{
    template <>
    struct ExtraDistributionDataData<PaludisDistribution>
    {
        static std::string config_file_name()
        {
            return "paludis.conf";
        }

        static std::shared_ptr<PaludisDistribution> make_data(const std::shared_ptr<const KeyValueConfigFile> & k)
        {
            return std::make_shared<PaludisDistribution>(make_named_values<PaludisDistribution>(
                            n::bashrc_filename() = k->get("bashrc_filename"),
                            n::info_messages_are_spam() = destringify<bool>(k->get("info_messages_are_spam")),
                            n::keywords_filename_part() = k->get("keywords_filename_part"),
                            n::licenses_filename_part() = k->get("licenses_filename_part"),
                            n::mandatory_userpriv() = destringify<bool>(k->get("mandatory_userpriv")),
                            n::mirrors_filename_part() = k->get("mirrors_filename_part"),
                            n::output_filename_part() = k->get("output_filename_part"),
                            n::output_managers_directory() = k->get("output_managers_directory"),
                            n::package_mask_filename_part() = k->get("package_mask_filename_part"),
                            n::package_unmask_filename_part() = k->get("package_unmask_filename_part"),
                            n::repositories_directory() = k->get("repositories_directory"),
                            n::repository_defaults_filename_part() = k->get("repository_defaults_filename_part"),
                            n::suggestions_filename_part() = k->get("suggestions_filename_part"),
                            n::use_filename_part() = k->get("use_filename_part")
                            ));
        }
    };
}

namespace paludis
{
    template class ExtraDistributionData<PaludisDistribution>;
    template class Singleton<ExtraDistributionData<PaludisDistribution>>;
}
