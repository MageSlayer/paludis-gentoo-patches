/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

namespace paludis
{
    /**
     * Thrown if an error occurs whilst building a DepList.
     *
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class DepListError : public Exception
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
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class AllMaskedError : public DepListError
    {
        private:
            const PackageDepSpec _query;

        public:
            ///\name Basic operations
            ///\{

            AllMaskedError(const PackageDepSpec & query) throw ();

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
    };

    /**
     * Thrown if all versions of a particular spec are masked,
     * but would not be if use requirements were not in effect.
     *
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class UseRequirementsNotMetError : public DepListError
    {
        private:
            std::string _query;

        public:
            ///\name Basic operations
            ///\{

            UseRequirementsNotMetError(const std::string & query) throw ();

            virtual ~UseRequirementsNotMetError() throw ()
            {
            }

            ///\}

            /**
             * Our query.
             */
            const std::string & query() const
            {
                return _query;
            }
    };

    /**
     * Thrown if a downgrade is forced and we're not allowed to downgrade.
     *
     * \ingroup grpexceptions
     * \ingroup grpdepresolver
     * \nosubgrouping
     */
    class DowngradeNotAllowedError : public DepListError
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
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class BlockError : public DepListError
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
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class CircularDependencyError : public DepListError
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
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class NoDestinationError : public DepListError
    {
        public:
            ///\name Basic operations
            ///\{

            NoDestinationError(const PackageDatabaseEntry &,
                    std::tr1::shared_ptr<const DestinationsCollection>) throw ();

            ///\}
    };
}

#endif
