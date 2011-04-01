/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Mike Kelly
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

#include <paludis/resolver/match_qpns.hh>

#include <paludis/util/make_named_values.hh>

#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/package_dep_spec_properties.hh>
#include <paludis/name.hh>
#include <paludis/package_dep_spec_constraint.hh>

using namespace paludis;
using namespace paludis::resolver;

// The 's' is silent...
bool
paludis::resolver::match_qpns(
        const Environment &,
        const PackageDepSpec & spec,
        const QualifiedPackageName & package)
{
    /* decided nothing, so we can only work for cat/pkg, where
     * either can be wildcards (we could work for :slot too,
     * but we're lazy) */
    if (! package_dep_spec_has_properties(spec, make_named_values<PackageDepSpecProperties>(
                    n::has_additional_requirements() = false,
                    n::has_any_slot_requirement() = false,
                    n::has_category_name_part() = indeterminate,
                    n::has_exact_slot_requirement() = false,
                    n::has_from_repository() = false,
                    n::has_in_repository() = false,
                    n::has_installable_to_path() = false,
                    n::has_installable_to_repository() = false,
                    n::has_installed_at_path() = false,
                    n::has_key_requirements() = false,
                    n::has_package() = indeterminate,
                    n::has_package_name_part() = indeterminate,
                    n::has_tag() = false,
                    n::has_version_requirements() = false
                    )))
        return false;

    if (spec.package_name_constraint() && spec.package_name_constraint()->name() != package)
        return false;
    if (spec.package_name_part_constraint() && spec.package_name_part_constraint()->name_part() != package.package())
        return false;
    if (spec.category_name_part_constraint() && spec.category_name_part_constraint()->name_part() != package.category())
        return false;

    return true;
}

