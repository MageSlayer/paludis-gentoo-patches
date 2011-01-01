/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_PROPERTIES_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DEP_SPEC_PROPERTIES_HH 1

#include <paludis/package_dep_spec_properties-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/tribool.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_has_additional_requirements> has_additional_requirements;
        typedef Name<struct name_has_category_name_part> has_category_name_part;
        typedef Name<struct name_has_from_repository> has_from_repository;
        typedef Name<struct name_has_in_repository> has_in_repository;
        typedef Name<struct name_has_installable_to_path> has_installable_to_path;
        typedef Name<struct name_has_installable_to_repository> has_installable_to_repository;
        typedef Name<struct name_has_installed_at_path> has_installed_at_path;
        typedef Name<struct name_has_package> has_package;
        typedef Name<struct name_has_package_name_part> has_package_name_part;
        typedef Name<struct name_has_slot_requirement> has_slot_requirement;
        typedef Name<struct name_has_tag> has_tag;
        typedef Name<struct name_has_version_requirements> has_version_requirements;
    }

    /**
     * Parameters for package_dep_spec_has_properties.
     *
     * \since 0.34.1
     * \ingroup g_dep_spec
     */
    struct PackageDepSpecProperties
    {
        NamedValue<n::has_additional_requirements, Tribool> has_additional_requirements;
        NamedValue<n::has_category_name_part, Tribool> has_category_name_part;
        NamedValue<n::has_from_repository, Tribool> has_from_repository;
        NamedValue<n::has_in_repository, Tribool> has_in_repository;
        NamedValue<n::has_installable_to_path, Tribool> has_installable_to_path;
        NamedValue<n::has_installable_to_repository, Tribool> has_installable_to_repository;
        NamedValue<n::has_installed_at_path, Tribool> has_installed_at_path;
        NamedValue<n::has_package, Tribool> has_package;
        NamedValue<n::has_package_name_part, Tribool> has_package_name_part;
        NamedValue<n::has_slot_requirement, Tribool> has_slot_requirement;
        NamedValue<n::has_tag, Tribool> has_tag;
        NamedValue<n::has_version_requirements, Tribool> has_version_requirements;
    };
}

#endif
