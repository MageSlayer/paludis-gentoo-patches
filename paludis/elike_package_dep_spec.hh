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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_HH 1

#include <paludis/elike_package_dep_spec-fwd.hh>
#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct add_package_requirement_name> add_package_requirement;
        typedef Name<struct add_version_requirement_name> add_version_requirement;
        typedef Name<struct check_sanity_name> check_sanity;
        typedef Name<struct get_remove_trailing_version_name> get_remove_trailing_version;
        typedef Name<struct get_remove_version_operator_name> get_remove_version_operator;
        typedef Name<struct has_version_operator_name> has_version_operator;
        typedef Name<struct options_for_partially_made_package_dep_spec_name> options_for_partially_made_package_dep_spec;
        typedef Name<struct remove_trailing_repo_if_exists_name> remove_trailing_repo_if_exists;
        typedef Name<struct remove_trailing_slot_if_exists_name> remove_trailing_slot_if_exists;
        typedef Name<struct remove_trailing_square_bracket_if_exists_name> remove_trailing_square_bracket_if_exists;
    }

    struct GenericELikePackageDepSpecParseFunctions
    {
        NamedValue<n::add_package_requirement, std::tr1::function<void (const std::string &, PartiallyMadePackageDepSpec &)> > add_package_requirement;
        NamedValue<n::add_version_requirement, std::tr1::function<void (const VersionOperator &, const VersionSpec &, PartiallyMadePackageDepSpec &)> > add_version_requirement;
        NamedValue<n::check_sanity, std::tr1::function<void (const std::string &)> > check_sanity;
        NamedValue<n::get_remove_trailing_version, std::tr1::function<VersionSpec (std::string &)> > get_remove_trailing_version;
        NamedValue<n::get_remove_version_operator, std::tr1::function<VersionOperator (std::string &)> > get_remove_version_operator;
        NamedValue<n::has_version_operator, std::tr1::function<bool (const std::string &)> > has_version_operator;
        NamedValue<n::options_for_partially_made_package_dep_spec, std::tr1::function<const PartiallyMadePackageDepSpecOptions ()> > options_for_partially_made_package_dep_spec;
        NamedValue<n::remove_trailing_repo_if_exists, std::tr1::function<void (std::string &, PartiallyMadePackageDepSpec &)> > remove_trailing_repo_if_exists;
        NamedValue<n::remove_trailing_slot_if_exists, std::tr1::function<void (std::string &, PartiallyMadePackageDepSpec &)> > remove_trailing_slot_if_exists;
        NamedValue<n::remove_trailing_square_bracket_if_exists, std::tr1::function<bool (std::string &, PartiallyMadePackageDepSpec &)> > remove_trailing_square_bracket_if_exists;
    };
}

#endif
