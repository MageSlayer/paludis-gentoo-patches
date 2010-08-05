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

#ifndef PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_ELIKE_PACKAGE_DEP_SPEC_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/options-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/version_operator-fwd.hh>
#include <paludis/version_spec-fwd.hh>
#include <paludis/partially_made_package_dep_spec-fwd.hh>
#include <functional>
#include <iosfwd>

namespace paludis
{

#include <paludis/elike_package_dep_spec-se.hh>

    typedef Options<ELikePackageDepSpecOption> ELikePackageDepSpecOptions;

    struct GenericELikePackageDepSpecParseFunctions;

    PackageDepSpec parse_generic_elike_package_dep_spec(const std::string & ss, const GenericELikePackageDepSpecParseFunctions & fns)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    PartiallyMadePackageDepSpec partial_parse_generic_elike_package_dep_spec(const std::string & ss,
            const GenericELikePackageDepSpecParseFunctions & fns)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    PackageDepSpec parse_elike_package_dep_spec(const std::string & ss, const ELikePackageDepSpecOptions &,
            const VersionSpecOptions &,
            const std::shared_ptr<const PackageID> &)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    PartiallyMadePackageDepSpec partial_parse_elike_package_dep_spec(const std::string & ss,
            const ELikePackageDepSpecOptions &,
            const VersionSpecOptions &,
            const std::shared_ptr<const PackageID> &)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    void elike_check_sanity(const std::string & s) PALUDIS_VISIBLE;

    bool elike_remove_trailing_square_bracket_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
            const ELikePackageDepSpecOptions & options,
            const VersionSpecOptions & version_options,
            bool & had_bracket_version_requirements,
            bool & had_use_requirements, const std::shared_ptr<const PackageID> & id) PALUDIS_VISIBLE;

    void elike_remove_trailing_repo_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
            const ELikePackageDepSpecOptions & options) PALUDIS_VISIBLE;

    void elike_remove_trailing_slot_if_exists(std::string & s, PartiallyMadePackageDepSpec & result,
            const ELikePackageDepSpecOptions & options) PALUDIS_VISIBLE;

    bool elike_has_version_operator(const std::string & s, const bool had_bracket_version_requirements,
            const ELikePackageDepSpecOptions & options) PALUDIS_VISIBLE;

    VersionOperator elike_get_remove_version_operator(std::string & s,
            const ELikePackageDepSpecOptions & options) PALUDIS_VISIBLE;

    VersionSpec elike_get_remove_trailing_version(std::string & s,
            const VersionSpecOptions &) PALUDIS_VISIBLE;

    void elike_add_version_requirement(const VersionOperator & op, const VersionSpec & spec, PartiallyMadePackageDepSpec & result)
        PALUDIS_VISIBLE;

    void elike_add_package_requirement(const std::string & s, PartiallyMadePackageDepSpec & result) PALUDIS_VISIBLE;
}

#endif
