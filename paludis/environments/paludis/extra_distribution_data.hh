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

#ifndef PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_EXTRA_DISTRIBUTION_DATA_HH
#define PALUDIS_GUARD_PALUDIS_ENVIRONMENTS_PALUDIS_EXTRA_DISTRIBUTION_DATA_HH 1

#include <paludis/distribution-fwd.hh>
#include <paludis/util/named_value.hh>
#include <string>

namespace paludis
{
    namespace n
    {
        struct bashrc_filename;
        struct info_messages_are_spam;
        struct keywords_filename_part;
        struct licenses_filename_part;
        struct mandatory_userpriv;
        struct mirrors_filename_part;
        struct output_filename_part;
        struct output_managers_directory;
        struct package_mask_filename_part;
        struct package_unmask_filename_part;
        struct repositories_directory;
        struct repository_defaults_filename_part;
        struct use_filename_part;
    }

    namespace paludis_environment
    {
        struct PaludisDistribution
        {
            NamedValue<n::bashrc_filename, std::string> bashrc_filename;
            NamedValue<n::info_messages_are_spam, bool> info_messages_are_spam;
            NamedValue<n::keywords_filename_part, std::string> keywords_filename_part;
            NamedValue<n::licenses_filename_part, std::string> licenses_filename_part;
            NamedValue<n::mandatory_userpriv, bool> mandatory_userpriv;
            NamedValue<n::mirrors_filename_part, std::string> mirrors_filename_part;
            NamedValue<n::output_filename_part, std::string> output_filename_part;
            NamedValue<n::output_managers_directory, std::string> output_managers_directory;
            NamedValue<n::package_mask_filename_part, std::string> package_mask_filename_part;
            NamedValue<n::package_unmask_filename_part, std::string> package_unmask_filename_part;
            NamedValue<n::repositories_directory, std::string> repositories_directory;
            NamedValue<n::repository_defaults_filename_part, std::string> repository_defaults_filename_part;
            NamedValue<n::use_filename_part, std::string> use_filename_part;
        };

        typedef ExtraDistributionData<PaludisDistribution> PaludisExtraDistributionData;
    }
}


#endif
