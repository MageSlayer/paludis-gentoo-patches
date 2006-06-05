/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/dep_atom.hh>
#include <paludis/name.hh>
#include <paludis/repository.hh>
#include <paludis/util/counted_ptr.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/join.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/smart_record.hh>
#include <paludis/util/collection.hh>
#include <paludis/version_metadata.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/contents.hh>

#include <ostream>
#include <algorithm>
#include <functional>
#include <iterator>

/** \file
 * Declarations for the PackageDatabase class and related utilities.
 *
 * \ingroup grppackagedatabase
 */

namespace paludis
{
    class PackageDepAtom;
    class Environment;

    /**
     * A PackageDatabaseError is an error that occurs when performing some
     * operation upon a PackageDatabase.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class PackageDatabaseError : public Exception
    {
        protected:
            /**
             * Constructor.
             */
            PackageDatabaseError(const std::string & message) throw ();
    };

    /**
     * A PackageDatabaseLookupError descendent is thrown if an error occurs
     * when looking for something in a PackageDatabase.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class PackageDatabaseLookupError : public PackageDatabaseError
    {
        protected:
            /**
             * Constructor.
             */
            PackageDatabaseLookupError(const std::string & message) throw ();
    };

    /**
     * Thrown if a PackageDatabase query results in more than one matching
     * Package.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class AmbiguousPackageNameError : public PackageDatabaseLookupError
    {
        private:
            std::string _name;
            std::list<std::string> _names;

        public:
            /**
             * Constructor.
             */
            template <typename I_>
            AmbiguousPackageNameError(const std::string & name,
                    I_ begin, const I_ end) throw ();

            /**
             * Destructor.
             */
            virtual ~AmbiguousPackageNameError() throw ()
            {
            }

            /**
             * The name of the package.
             */
            const std::string & name() const
            {
                return _name;
            }

            /**
             * Iterator over possible matches.
             */
            typedef std::list<std::string>::const_iterator OptionsIterator;

            /**
             * Start of our options.
             */
            OptionsIterator begin_options() const
            {
                return _names.begin();
            }

            /**
             * Past the end of our options.
             */
            OptionsIterator end_options() const
            {
                return _names.end();
            }
    };

    template <typename I_>
    AmbiguousPackageNameError::AmbiguousPackageNameError(const std::string & name,
            I_ begin, const I_ end) throw () :
        PackageDatabaseLookupError("Ambiguous package name '" + name + "' (candidates are " +
            join(begin, end, ", ") + ")"),
        _name(name)
    {
        std::transform(begin, end, std::back_inserter(_names),
                &stringify<typename std::iterator_traits<I_>::value_type>);
    }

    /**
     * Thrown if a Repository with the same name as an existing member is added
     * to a PackageDatabase.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class DuplicateRepositoryError : public PackageDatabaseError
    {
        public:
            /**
             * Constructor.
             */
            DuplicateRepositoryError(const std::string & name) throw ();
    };

    /**
     * Thrown if there is no Package in a PackageDatabase with the given
     * name.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class NoSuchPackageError : public PackageDatabaseLookupError
    {
        private:
            std::string _name;

        public:
            /**
             * Constructor.
             */
            NoSuchPackageError(const std::string & name) throw ();

            /**
             * Destructor.
             */
            virtual ~NoSuchPackageError() throw ()
            {
            }

            /**
             * Name of the package.
             */
            const std::string & name() const
            {
                return _name;
            }
    };

    /**
     * Thrown if there is no Repository in a RepositoryDatabase with the given
     * name.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class NoSuchRepositoryError : public PackageDatabaseLookupError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchRepositoryError(const std::string & name) throw ();
    };

    /**
     * Thrown if there is no Version in a PackageDatabase with the given
     * name.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class NoSuchVersionError : public PackageDatabaseLookupError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchVersionError(const std::string & pkg_name,
                    const VersionSpec & version) throw ();
    };

    /**
     * Do we want installed, uninstalled or either when querying?
     *
     * \ingroup grppackagedatabase
     */
    enum InstallState
    {
        is_installed_only,    ///< Installed only
        is_uninstalled_only,  ///< Uninstalled only
        is_either             ///< Either
    };

    /**
     * A PackageDatabase can be queried for Package instances.
     *
     * \ingroup grppackagedatabase
     */
    class PackageDatabase :
        private PrivateImplementationPattern<PackageDatabase>,
        private InstantiationPolicy<PackageDatabase, instantiation_method::NonCopyableTag>,
        public InternalCounted<PackageDatabase>
    {
        private:
            PackageDatabaseEntryCollection::Pointer _do_query(
                    const PackageDepAtom & a, const InstallState) const;

        public:
            /**
             * Constructor.
             */
            explicit PackageDatabase(const Environment * const);

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
             * Fetch the contents for a particular item.
             */
            Contents::ConstPointer fetch_contents(const PackageDatabaseEntry &) const;

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
                    const PackageDepAtom & a,
                    const InstallState) const;

            /**
             * Query the repository (overload for a CountedPtr)
             */
            PackageDatabaseEntryCollection::Pointer query(
                    PackageDepAtom::ConstPointer a,
                    const InstallState) const;

            /**
             * Which repository is better?
             */
            const RepositoryName & better_repository(const RepositoryName &,
                    const RepositoryName &) const;

            /**
             * Iterate over all of our repositories.
             */
            typedef std::list<Repository::ConstPointer>::const_iterator RepositoryIterator;

            /**
             * Iterator to the start of our repositories.
             */
            RepositoryIterator begin_repositories() const;

            /**
             * Iterator to past the end of our repositories.
             */
            RepositoryIterator end_repositories() const;
    };
}

#endif
