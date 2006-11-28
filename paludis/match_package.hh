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
     * For internal use by match_package.
     *
     * \ingroup grpmatchpackage
     */
    namespace match_package_internals
    {
        /**
         * Do the match on a PackageDatabaseEntry.
         *
         * \ingroup grpmatchpackage
         */
        bool do_match(
                const Environment * const env,
                const PackageDepAtom * const atom,
                const PackageDatabaseEntry * const entry)
            PALUDIS_ATTRIBUTE((nonnull(1, 2, 3)));

        /**
         * Normalise env type.
         *
         * \ingroup grpmatchpackage
         */
        inline const Environment * sanitise_env(const Environment * env)
        {
            return env;
        }

        /**
         * Normalise DB type.
         *
         * \ingroup grpmatchpackage
         */
        template <typename P1_>
        inline const Environment * sanitise_env(const CountedPtr<const Environment, P1_> env)
        {
            return env.raw_pointer();
        }

        /**
         * Normalise atom type.
         *
         * \ingroup grpmatchpackage
         */
        inline const PackageDepAtom * sanitise_atom(const PackageDepAtom * atom)
        {
            return atom;
        }

        /**
         * Normalise atom type.
         *
         * \ingroup grpmatchpackage
         */
        inline const PackageDepAtom * sanitise_atom(const PackageDepAtom & atom)
        {
            return &atom;
        }

        /**
         * Normalise atom type.
         *
         * \ingroup grpmatchpackage
         */
        template <typename P1_>
        inline const PackageDepAtom * sanitise_atom(const CountedPtr<const PackageDepAtom, P1_> atom)
        {
            return atom.raw_pointer();
        }

        /**
         * Normalise target type.
         *
         * \ingroup grpmatchpackage
         */
        template <typename T_>
        inline const T_ * sanitise_target(const T_ * e)
        {
            return e;
        }

        /**
         * Normalise target type.
         *
         * \ingroup grpmatchpackage
         */
        template <typename T_>
        inline const T_ * sanitise_target(const T_ & e)
        {
            return &e;
        }
    }

    /**
     * Return whether the specified atom matches the specified target.
     *
     * \param env     Some kind of environment
     * \param atom    Some kind of package dep atom
     * \param target  Some kind of target
     *
     * \ingroup grpmatchpackage
     */
    template <typename Env_, typename Atom_, typename Target_>
    bool match_package(
            const Env_ & env,
            const Atom_ & atom,
            const Target_ & target)
    {
        return match_package_internals::do_match(
                match_package_internals::sanitise_env(env),
                match_package_internals::sanitise_atom(atom),
                match_package_internals::sanitise_target(target));
    }
}

#endif
