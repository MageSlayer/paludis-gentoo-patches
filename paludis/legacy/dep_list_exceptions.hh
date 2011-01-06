/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_EXCEPTIONS_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_EXCEPTIONS_HH 1

#include <paludis/util/exception.hh>
#include <paludis/package_database.hh>
#include <paludis/environment.hh>

/** \file
 * Declarations for DepList exceptions.
 *
 * \ingroup g_dep_spec
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Thrown if an error occurs whilst building a DepList.
     *
     * \ingroup g_dep_list
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepListError : public Exception
    {
        protected:
            ///\name Basic operations
            ///\{

            DepListError(const std::string &) throw ();

            ///\}
    };

    /**
     * Thrown if all versions of a particular spec are masked.
     *
     * \ingroup g_dep_list
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AllMaskedError : public DepListError
    {
        private:
            const PackageDepSpec _query;
            const std::shared_ptr<const PackageID> _from_id;

        public:
            ///\name Basic operations
            ///\{

            AllMaskedError(const PackageDepSpec & query, const std::shared_ptr<const PackageID> & id) throw ();

            virtual ~AllMaskedError() throw ()
            {
            }

            ///\}

            /**
             * Our query.
             */
            const PackageDepSpec query() const
            {
                return _query;
            }

            const std::shared_ptr<const PackageID> from_id() const
            {
                return _from_id;
            }
    };

    /**
     * Thrown if all versions of a particular spec are masked,
     * but would not be if additional requirements were not in effect.
     *
     * \ingroup g_dep_list
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE AdditionalRequirementsNotMetError : public DepListError
    {
        private:
            PackageDepSpec _query;
            const std::shared_ptr<const PackageID> _from_id;
            const std::shared_ptr<const PackageID> _id;

        public:
            ///\name Basic operations
            ///\{

            AdditionalRequirementsNotMetError(
                    const PackageDepSpec & query,
                    const std::shared_ptr<const PackageID> & from_id,
                    const std::shared_ptr<const PackageID> & id) throw ();

            virtual ~AdditionalRequirementsNotMetError() throw ();

            ///\}

            /**
             * Our query.
             */
            const PackageDepSpec query() const
            {
                return _query;
            }

            /**
             * Our ID of choice.
             *
             * \since 0.44
             */
            const std::shared_ptr<const PackageID> package_id() const
            {
                return _id;
            }

            const std::shared_ptr<const PackageID> from_package_id() const
            {
                return _from_id;
            }
    };

    /**
     * Thrown if a downgrade is forced and we're not allowed to downgrade.
     *
     * \ingroup g_exceptions
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DowngradeNotAllowedError : public DepListError
    {
        public:
            ///\name Basic operations
            ///\{

            DowngradeNotAllowedError(const std::string & to, const std::string & from) throw ();

            virtual ~DowngradeNotAllowedError() throw ();

            ///\}
    };

    /**
     * Thrown if a block is encountered.
     *
     * \ingroup g_exceptions
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE BlockError : public DepListError
    {
        public:
            ///\name Basic operations
            ///\{

            BlockError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if a circular dependency is encountered.
     *
     * \ingroup g_dep_list
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE CircularDependencyError : public DepListError
    {
        public:
            ///\name Basic operations
            ///\{

            CircularDependencyError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Thrown if no destination can be found.
     *
     * \ingroup g_dep_list
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE NoDestinationError : public DepListError
    {
        public:
            ///\name Basic operations
            ///\{

            NoDestinationError(const std::shared_ptr<const PackageID> &,
                    const std::shared_ptr<const DestinationsSet> &) throw ();

            ///\}
    };
}

#endif
