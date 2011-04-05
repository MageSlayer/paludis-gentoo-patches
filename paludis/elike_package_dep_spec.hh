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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_HH 1

#include <paludis/elike_package_dep_spec-fwd.hh>
#include <paludis/dep_spec_data-fwd.hh>

#include <paludis/util/named_value.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_add_package_requirement> add_package_requirement;
        typedef Name<struct name_add_version_requirement> add_version_requirement;
        typedef Name<struct name_check_sanity> check_sanity;
        typedef Name<struct name_get_remove_trailing_version> get_remove_trailing_version;
        typedef Name<struct name_get_remove_version_operator> get_remove_version_operator;
        typedef Name<struct name_has_version_operator> has_version_operator;
        typedef Name<struct name_options_for_partially_made_package_dep_spec> options_for_partially_made_package_dep_spec;
        typedef Name<struct name_remove_trailing_repo_if_exists> remove_trailing_repo_if_exists;
        typedef Name<struct name_remove_trailing_slot_if_exists> remove_trailing_slot_if_exists;
        typedef Name<struct name_remove_trailing_square_bracket_if_exists> remove_trailing_square_bracket_if_exists;
    }

    struct GenericELikePackageDepSpecParseFunctions
    {
        NamedValue<n::add_package_requirement, std::function<void (const std::string &, MutablePackageDepSpecData &)> > add_package_requirement;
        NamedValue<n::add_version_requirement, std::function<void (
                const VersionSpec &,
                const VersionOperator &,
                const VersionConstraintCombiner,
                MutablePackageDepSpecData &)> > add_version_requirement;
        NamedValue<n::check_sanity, std::function<void (const std::string &)> > check_sanity;
        NamedValue<n::get_remove_trailing_version, std::function<VersionSpec (std::string &)> > get_remove_trailing_version;
        NamedValue<n::get_remove_version_operator, std::function<VersionOperator (std::string &)> > get_remove_version_operator;
        NamedValue<n::has_version_operator, std::function<bool (const std::string &)> > has_version_operator;
        NamedValue<n::options_for_partially_made_package_dep_spec, std::function<const PackageDepSpecDataOptions ()> > options_for_partially_made_package_dep_spec;
        NamedValue<n::remove_trailing_repo_if_exists, std::function<void (std::string &, MutablePackageDepSpecData &)> > remove_trailing_repo_if_exists;
        NamedValue<n::remove_trailing_slot_if_exists, std::function<void (std::string &, MutablePackageDepSpecData &)> > remove_trailing_slot_if_exists;
        NamedValue<n::remove_trailing_square_bracket_if_exists, std::function<bool (std::string &, MutablePackageDepSpecData &)> > remove_trailing_square_bracket_if_exists;
    };
}

#endif
