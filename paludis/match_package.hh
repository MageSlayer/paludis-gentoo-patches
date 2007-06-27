/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH 1

/** \file
 * Declare the match_package function.
 *
 * Do not merge this file into dep_spec. It will cause all sorts of horrible
 * circular dependency issues. Avoid including this file in headers if at all
 * possible.
 *
 * \ingroup grpmatchpackage
 */

#include <paludis/util/attributes.hh>
#include <paludis/util/tr1_memory.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>

namespace paludis
{
    /**
     * Return whether the specified spec matches the specified target.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package(
            const Environment & env,
            const PackageDepSpec & spec,
            const PackageID & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Return whether the specified spec matches the specified target, for heirarchies.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package_in_set(
            const Environment & env,
            const SetSpecTree::ConstItem & spec,
            const PackageID & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
