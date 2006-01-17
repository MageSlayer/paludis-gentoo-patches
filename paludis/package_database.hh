/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_HH
#define PALUDIS_GUARD_PALUDIS_PACKAGE_DATABASE_HH 1

#include <paludis/instantiation_policy.hh>
#include <paludis/private_implementation_pattern.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/repository.hh>
#include <paludis/version_metadata.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/package_database_entry_collection.hh>
#include <paludis/package_dep_atom.hh>
#include <paludis/repository.hh>

namespace paludis
{
    class PackageDepAtom;
    class PackageDatabase;

    /**
     * A PackageDatabase can be queried for Package instances.
     */
    class PackageDatabase :
        private PrivateImplementationPattern<PackageDatabase>,
        private InstantiationPolicy<PackageDatabase, instantiation_method::NonCopyableTag>,
        public InternalCounted<PackageDatabase>
    {
        public:
            /**
             * Constructor.
             */
            PackageDatabase();

            /**
             * Destructor.
             */
            ~PackageDatabase();

            /**
             * Add a repository.
             *
             * \exception DuplicateRepositoryError if a Repository with the
             * same name as the new Repository already exists in our
             * collection.
             */
            void add_repository(Repository::ConstPointer);

            /**
             * Fetch the version metadata for a particular item.
             */
            VersionMetadata::ConstPointer fetch_metadata(const PackageDatabaseEntry &) const;

            /**
             * Fetch a named repository.
             */
            Repository::ConstPointer fetch_repository(const RepositoryName &) const;

            /**
             * Fetch the name of our 'favourite' repository (if a repository's
             * name matches this when doing a graphical display, the repository
             * name part may be omitted).
             */
            RepositoryName favourite_repository() const;

            /**
             * Disambiguate a package name.
             */
            QualifiedPackageName fetch_unique_qualified_package_name(
                    const PackageNamePart &) const;

            /**
             * Query the repository.
             */
            PackageDatabaseEntryCollection::Pointer query(
                    const PackageDepAtom * const a) const;

            /**
             * Query the repository (overload for a CountedPtr)
             */
            PackageDatabaseEntryCollection::Pointer query(
                    PackageDepAtom::ConstPointer a) const;

            /**
             * Which repository is better?
             */
            const RepositoryName & better_repository(const RepositoryName &,
                    const RepositoryName &) const;


            typedef std::list<Repository::ConstPointer>::const_iterator RepositoryIterator;

            RepositoryIterator begin_repositories() const;

            RepositoryIterator end_repositories() const;
    };
}

#endif
