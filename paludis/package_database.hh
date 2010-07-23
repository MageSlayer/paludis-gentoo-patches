/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/package_database-fwd.hh>
#include <paludis/dep_spec.hh>
#include <paludis/name.hh>
#include <paludis/repository.hh>
#include <paludis/selection-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/join.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/version_spec.hh>
#include <paludis/contents.hh>

#include <iosfwd>

/** \file
 * Declarations for PackageDatabase.
 *
 * \ingroup g_package_database
 *
 * \section Examples
 *
 * - \ref example_package_database.cc "example_package_database.cc"
 */

namespace paludis
{
    /**
     * A PackageDatabaseError is an error that occurs when performing some
     * operation upon a PackageDatabase.
     *
     * \ingroup g_exceptions
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE PackageDatabaseError :
        public Exception
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
     * \ingroup g_exceptions
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE PackageDatabaseLookupError :
        public PackageDatabaseError
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
     * \ingroup g_exceptions
     * \ingroup g_package_database
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
            const std::string & name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over possible matches
            ///\{

            struct OptionsConstIteratorTag;
            typedef WrappedForwardIterator<OptionsConstIteratorTag,
                    const std::string> OptionsConstIterator;

            OptionsConstIterator begin_options() const PALUDIS_ATTRIBUTE((warn_unused_result));
            OptionsConstIterator end_options() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    /**
     * Thrown if a Repository with the same name as an existing member is added
     * to a PackageDatabase.
     *
     * \ingroup g_exceptions
     * \ingroup g_package_database
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DuplicateRepositoryError :
        public PackageDatabaseError
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
     * \ingroup g_exceptions
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE NoSuchPackageError :
        public PackageDatabaseLookupError
    {
        private:
            std::string _name;

        public:
            ///\name Basic operations
            ///\{

            NoSuchPackageError(const std::string & name) throw ();

            virtual ~NoSuchPackageError() throw ()
            {
            }

            ///\}

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
     * \ingroup g_exceptions
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE NoSuchRepositoryError : public PackageDatabaseLookupError
    {
        private:
            const RepositoryName _name;

        public:
            ///\name Basic operations
            ///\{

            NoSuchRepositoryError(const RepositoryName &) throw ();

            ~NoSuchRepositoryError() throw ();

            ///\}

            /**
             * The name of our repository.
             */
            RepositoryName name() const;
    };

    /**
     * A PackageDatabase, which is owned by an Environment, contains a number of
     * Repository instances and supports various querying methods.
     *
     * \ingroup g_package_database
     */
    class PALUDIS_VISIBLE PackageDatabase :
        private Pimp<PackageDatabase>
    {
        private:
            static const Filter & all_filter() PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            explicit PackageDatabase(const Environment * const);

            ~PackageDatabase();

            PackageDatabase(const PackageDatabase &) = delete;
            PackageDatabase & operator= (const PackageDatabase &) = delete;

            ///\}

            /**
             * Add a repository.
             *
             * \exception DuplicateRepositoryError if a Repository with the
             * same name as the new Repository already exists in our
             * collection.
             */
            void add_repository(int importance, const std::shared_ptr<Repository>);

            /**
             * Fetch a named repository.
             */
            std::shared_ptr<const Repository> fetch_repository(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch a named repository.
             */
            std::shared_ptr<Repository> fetch_repository(const RepositoryName &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Do we have a named repository?
             */
            bool has_repository_named(const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch the name of our 'favourite' repository (if a repository's
             * name matches this when doing a graphical display, the repository
             * name part may be omitted).
             *
             * Note that this is the repository with the <i>lowest</i> importance
             * that is not a virtuals or installed_virtuals repository.
             */
            RepositoryName favourite_repository() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Disambiguate a package name.  If a filter is specified,
             * limit the potential results to packages that match.
             *
             * \throw AmbiguousPackageNameError if there is no unambiguous
             * disambiguation.
             */
            QualifiedPackageName fetch_unique_qualified_package_name(
                    const PackageNamePart &, const Filter & = all_filter()) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Return true if the first repository is more important than the second.
             */
            bool more_important_than(const RepositoryName &, const RepositoryName &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name Iterate over our repositories
            ///\{

            struct RepositoryConstIteratorTag;
            typedef WrappedForwardIterator<RepositoryConstIteratorTag, const std::shared_ptr<Repository> > RepositoryConstIterator;

            RepositoryConstIterator begin_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            RepositoryConstIterator end_repositories() const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    extern template class WrappedForwardIterator<PackageDatabase::RepositoryConstIteratorTag, const std::shared_ptr<Repository> >;
    extern template class WrappedForwardIterator<AmbiguousPackageNameError::OptionsConstIteratorTag, const std::string>;
}

#endif
