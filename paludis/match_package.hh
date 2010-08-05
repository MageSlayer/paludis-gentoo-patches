/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH 1

/** \file
 * Declarations for match_package and match_package_in_set.
 *
 * \ingroup g_query
 *
 * \section Examples
 *
 * - \ref example_match_package.cc "example_match_package.cc"
 */

#include <paludis/match_package-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/spec_tree-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/changed_choices-fwd.hh>

namespace paludis
{
    /**
     * Return whether the specified PackageID matches the specified
     * PackageDepSpec.
     *
     * \ingroup g_query
     */
    bool match_package(
            const Environment & env,
            const PackageDepSpec & spec,
            const PackageID & target,
            const MatchPackageOptions & options)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Return whether the specified PackageID matches the specified
     * PackageDepSpec, with the specified ChangedChoices applied to the target
     * and the ID from which the dep came.
     *
     * \ingroup g_query
     * \since 0.51
     */
    bool match_package_with_maybe_changes(
            const Environment & env,
            const PackageDepSpec & spec,
            const ChangedChoices * const maybe_changes_to_owner,
            const PackageID & target,
            const ChangedChoices * const maybe_changes_to_target,
            const MatchPackageOptions & options)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Return whether the specified PackageID matches any of the items in the
     * specified set.
     *
     * Named sets inside the set are expanded.
     *
     * \ingroup g_query
     */
    bool match_package_in_set(
            const Environment & env,
            const SetSpecTree & spec,
            const PackageID & target,
            const MatchPackageOptions & options)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
