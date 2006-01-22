/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH
#define PALUDIS_GUARD_PALUDIS_MATCH_PACKAGE_HH 1

/** \file
 * Declare the match_package function.
 *
 * Do not merge this file into dep_atom. It will cause all sorts of horrible
 * circular dependency issues. Avoid including this file in headers if at all
 * possible.
 */

#include <paludis/package_database.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/dep_list.hh>

namespace paludis
{
    namespace match_package_internals
    {
        bool do_match(
                const PackageDatabase * const db,
                const PackageDepAtom * const atom,
                const PackageDatabaseEntry * const entry);

        bool do_match(
                const PackageDatabase * const db,
                const PackageDepAtom * const atom,
                const DepListEntry * const entry);

        inline const PackageDatabase * sanitise_db(const PackageDatabase * db)
        {
            return db;
        }

        inline const PackageDatabase * sanitise_db(const PackageDatabase & db)
        {
            return &db;
        }

        template <typename P1_, typename P2_>
        inline const PackageDatabase * sanitise_db(const CountedPtr<const PackageDatabase, P1_, P2_> db)
        {
            return db.raw_pointer();
        }

        inline const PackageDatabase * sanitise_db(const PackageDatabase::ConstPointer db)
        {
            return db.raw_pointer();
        }

        inline const PackageDepAtom * sanitise_atom(const PackageDepAtom * atom)
        {
            return atom;
        }

        inline const PackageDepAtom * sanitise_atom(const PackageDepAtom & atom)
        {
            return &atom;
        }

        template <typename P1_, typename P2_>
        inline const PackageDepAtom * sanitise_atom(const CountedPtr<const PackageDepAtom, P1_, P2_> atom)
        {
            return atom.raw_pointer();
        }

        template <typename T_>
        inline const T_ * sanitise_target(const T_ * e)
        {
            return e;
        }

        template <typename T_>
        inline const T_ * sanitise_target(const T_ & e)
        {
            return &e;
        }
    }

    template <typename DB_, typename Atom_, typename Target_>
    bool match_package(
            const DB_ & db,
            const Atom_ & atom,
            const Target_ & target)
    {
        return match_package_internals::do_match(
                match_package_internals::sanitise_db(db),
                match_package_internals::sanitise_atom(atom),
                match_package_internals::sanitise_target(target));
    }
}

#endif
