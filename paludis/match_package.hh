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

namespace paludis
{
    class Environment;
    class DepSpec;
    class PackageDepSpec;
    class PackageDatabaseEntry;

    /**
     * Return whether the specified spec matches the specified target.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package(
            const Environment & env,
            const PackageDepSpec & spec,
            const PackageDatabaseEntry & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;

    /**
     * Return whether the specified spec matches the specified target, for heirarchies.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package_in_heirarchy(
            const Environment & env,
            const DepSpec & spec,
            const PackageDatabaseEntry & target)
        PALUDIS_ATTRIBUTE((warn_unused_result)) PALUDIS_VISIBLE;
}

#endif
