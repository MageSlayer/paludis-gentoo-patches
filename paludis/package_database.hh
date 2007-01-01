/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/collection.hh>
#include <paludis/version_metadata.hh>
#include <paludis/version_spec.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/contents.hh>

#include <iosfwd>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

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
    class PALUDIS_VISIBLE PackageDatabaseError : public Exception
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
    class PALUDIS_VISIBLE PackageDatabaseLookupError : public PackageDatabaseError
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
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AmbiguousPackageNameError :
        public PackageDatabaseLookupError
    {
        private:
            struct NameData;
            NameData * const _name_data;

            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            template <typename I_>
            AmbiguousPackageNameError(const std::string & name,
                    I_ begin, const I_ end) throw ();

            AmbiguousPackageNameError(const AmbiguousPackageNameError &);

            virtual ~AmbiguousPackageNameError() throw ();

            ///\}

            /**
             * The name of the package.
             */
            const std::string & name() const;

            ///\name Iterate over possible matches
            ///\{

            typedef libwrapiter::ForwardIterator<AmbiguousPackageNameError,
                    const std::string> OptionsIterator;

            OptionsIterator begin_options() const;
            OptionsIterator end_options() const;

            ///\}
    };

    /**
     * Thrown if a Repository with the same name as an existing member is added
     * to a PackageDatabase.
     *
     * \ingroup grpexceptions
     * \ingroup grppackagedatabase
     */
    class PALUDIS_VISIBLE DuplicateRepositoryError : public PackageDatabaseError
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
    class PALUDIS_VISIBLE NoSuchPackageError : public PackageDatabaseLookupError
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
    class PALUDIS_VISIBLE NoSuchRepositoryError : public PackageDatabaseLookupError
    {
        public:
            /**
             * Constructor.
             */
            NoSuchRepositoryError(const std::string & name) throw ();
    };

    /**
     * Do we want installed, uninstalled or either when querying?
     *
     * \ingroup grppackagedatabase
     */
    enum InstallState
    {
        is_installed_only,    ///< Installed only
        is_installable_only,  ///< Installable only
        is_any,               ///< Either
        last_install_state
    };

    /**
     * A PackageDatabase can be queried for Package instances.
     *
     * \ingroup grppackagedatabase
     */
    class PALUDIS_VISIBLE PackageDatabase :
        private PrivateImplementationPattern<PackageDatabase>,
        private InstantiationPolicy<PackageDatabase, instantiation_method::NonCopyableTag>,
        public InternalCounted<PackageDatabase>
    {
        private:
           void _sort_package_database_entry_collection(
                   PackageDatabaseEntryCollection::Concrete &) const;

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
            void add_repository(Repository::Pointer);

            /**
             * Fetch a named repository.
             */
            Repository::ConstPointer fetch_repository(const RepositoryName &) const;

            /**
             * Fetch a named repository.
             */
            Repository::Pointer fetch_repository(const RepositoryName &);

            /**
             * Fetch the name of our 'favourite' repository (if a repository's
             * name matches this when doing a graphical display, the repository
             * name part may be omitted).
             *
             * Note that this is the repository with the <i>lowest</i> importance
             * that is not a virtuals or installed_virtuals repository.
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
             * Return true if the first repository is more important than the second.
             */
            bool more_important_than(const RepositoryName &, const RepositoryName &) const;

            ///\name Iterate over our repositories
            ///\{

            typedef libwrapiter::ForwardIterator<PackageDatabase, const Repository::Pointer> RepositoryIterator;

            RepositoryIterator begin_repositories() const;

            RepositoryIterator end_repositories() const;

            ///\}
    };

    /**
     * Write an InstallState to a stream.
     *
     * \ingroup grppackagedatabase
     */
    std::ostream &
    operator<< (std::ostream &, const InstallState &) PALUDIS_VISIBLE;
}

#endif
