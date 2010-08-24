/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_SELECTION_HH
#define PALUDIS_GUARD_PALUDIS_SELECTION_HH 1

#include <paludis/selection-fwd.hh>
#include <paludis/selection_handler-fwd.hh>
#include <paludis/filtered_generator-fwd.hh>
#include <paludis/filter-fwd.hh>
#include <paludis/generator-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/exception.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/environment-fwd.hh>

/** \file
 * Declarations for the Selection class.
 *
 * \ingroup g_selections
 *
 * \section Examples
 *
 * - \ref example_selection.cc "example_selection.cc"
 */

namespace paludis
{
    /**
     * Thrown if selection::RequireExactlyOne does not get exactly one result.
     *
     * \ingroup g_selections
     */
    class DidNotGetExactlyOneError :
        public Exception
    {
        public:
            DidNotGetExactlyOneError(const std::string &,
                    const std::shared_ptr<const PackageIDSet> &) throw ();
    };

    /**
     * A Selection subclass is passed to Environment::operator[] to obtain a set
     * of PackageID instances with given properties.
     *
     * Most Selection subclasses take a FilteredGenerator as a constructor
     * parameter that specifies the required properties; the Selection itself is
     * merely responsible for determining the format of the results.
     *
     * \ingroup g_selections
     */
    class PALUDIS_VISIBLE Selection :
        private Pimp<Selection>
    {
        protected:
            Selection(const std::shared_ptr<const SelectionHandler> &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Selection subclasses can be copied to a Selection without
             * destroying information.
             */
            Selection(const Selection &);

            ~Selection();
            Selection & operator= (const Selection &);

            ///\}

            /**
             * Return a string representation of our selection query.
             */
            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * For use by Environment, not to be called directly.
             */
            std::shared_ptr<PackageIDSequence> perform_select(const Environment * const) const
                PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    namespace selection
    {
        /**
         * Select some arbitrary version with the specified properties.
         *
         * Mostly used if you want to find out whether a PackageID with a
         * particular property exists, but do not want to do anything in
         * particular with the results.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE SomeArbitraryVersion :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                SomeArbitraryVersion(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return only the best version of each matching package with the
         * specified properties.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE BestVersionOnly :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                BestVersionOnly(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return the best version in each slot of each matching package with
         * the specified properties.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE BestVersionInEachSlot :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                BestVersionInEachSlot(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return all versions with the specified properties, sorted from worst
         * to best.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE AllVersionsSorted :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                AllVersionsSorted(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return all versions with the specified properties, sorted from worst
         * to best, but with like slots grouped together.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE AllVersionsGroupedBySlot :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                AllVersionsGroupedBySlot(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return all versions with the specified properties, in no particular
         * order.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE AllVersionsUnsorted :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                AllVersionsUnsorted(const FilteredGenerator &);

                ///\}
        };

        /**
         * Return the single version with the specified properties.
         *
         * If there is not exactly one version, throws DidNotGetExactlyOneError.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE RequireExactlyOne :
            public Selection
        {
            public:
                ///\name Basic operations
                ///\{

                RequireExactlyOne(const FilteredGenerator &);

                ///\}
        };
    }

    extern template class Pimp<Selection>;
}

#endif
