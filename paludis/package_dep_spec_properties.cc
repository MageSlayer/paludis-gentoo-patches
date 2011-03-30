/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#include <paludis/package_dep_spec_properties.hh>
#include <paludis/dep_spec.hh>
#include <paludis/util/sequence.hh>

using namespace paludis;

namespace
{
    inline bool check(const bool c, const Tribool w)
    {
        if (w.is_true())
            return c;
        else if (w.is_false())
            return ! c;
        else
            return true;
    }
}

bool
paludis::package_dep_spec_has_properties(const PackageDepSpec & spec, const PackageDepSpecProperties & properties)
{
    bool result(true);

    result = result && check(bool(spec.additional_requirements_ptr()) && ! spec.additional_requirements_ptr()->empty(), properties.has_additional_requirements());
    result = result && check(bool(spec.category_name_part_constraint()), properties.has_category_name_part());
    result = result && check(bool(spec.from_repository_constraint()), properties.has_from_repository());
    result = result && check(bool(spec.in_repository_constraint()), properties.has_in_repository());
    result = result && check(bool(spec.installable_to_path_constraint()), properties.has_installable_to_path());
    result = result && check(bool(spec.installable_to_repository_constraint()), properties.has_installable_to_repository());
    result = result && check(bool(spec.installed_at_path_constraint()), properties.has_installed_at_path());
    result = result && check(bool(spec.package_name_constraint()), properties.has_package());
    result = result && check(bool(spec.package_name_part_constraint()), properties.has_package_name_part());
    result = result && check(bool(spec.exact_slot_constraint()), properties.has_exact_slot_requirement());
    result = result && check(bool(spec.any_slot_constraint()), properties.has_any_slot_requirement());
    result = result && check(spec.version_requirements_ptr() && ! spec.version_requirements_ptr()->empty(), properties.has_version_requirements());

    return result;
}

