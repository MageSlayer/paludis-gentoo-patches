/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH 1

/** \file
 * Declare the match_package function.
 *
 * Do not merge this file into dep_atom. It will cause all sorts of horrible
 * circular dependency issues. Avoid including this file in headers if at all
 * possible.
 *
 * \ingroup grpmatchpackage
 */

#include <paludis/dep_atom.hh>
#include <paludis/environment.hh>
#include <paludis/util/attributes.hh>

namespace paludis
{
    /**
     * Return whether the specified atom matches the specified target.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package(
            const Environment & env,
            const PackageDepAtom & atom,
            const PackageDatabaseEntry & target);

    /**
     * Return whether the specified atom matches the specified target, for heirarchies.
     *
     * \ingroup grpmatchpackage
     */
    bool match_package_in_heirarchy(
            const Environment & env,
            const DepAtom & atom,
            const PackageDatabaseEntry & target);
}

#endif
