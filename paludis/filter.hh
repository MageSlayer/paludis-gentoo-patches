/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2014 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FILTER_HH
#define PALUDIS_GUARD_PALUDIS_FILTER_HH 1

#include <paludis/filter-fwd.hh>
#include <paludis/filter_handler-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/set-fwd.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/match_package-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <functional>
#include <memory>

/** \file
 * Declarations for the Filter class.
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
     * A Filter subclass can be used to further restrict the values picked by a
     * Generator, which when combined together produces a FilteredGenerator
     * which can be passed to a Selection subclass.
     *
     * \ingroup g_selections
     */
    class PALUDIS_VISIBLE Filter
    {
        private:
            Pimp<Filter> _imp;

        protected:
            Filter(const std::shared_ptr<const FilterHandler> &);

        public:
            ///\name Basic operations
            ///\{

            /**
             * Filter subclasses can be copied without losing information.
             */
            Filter(const Filter &);
            Filter & operator= (const Filter &);
            ~Filter();

            ///\}

            /**
             * A Filter can be represented as a string, for use by operator<<.
             */
            std::string as_string() const PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\name For use by Selection
            ///\{

            /**
             * Return any RepositoryContentMayExcludes that are implied by this
             * filter.
             *
             * \since 0.59
             */
            const RepositoryContentMayExcludes may_excludes() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Filter candidate repository names.
             */
            std::shared_ptr<const RepositoryNameSet> repositories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Filter candidate category names.
             */
            std::shared_ptr<const CategoryNamePartSet> categories(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const CategoryNamePartSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Filter candidate package names.
             */
            std::shared_ptr<const QualifiedPackageNameSet> packages(
                    const Environment * const,
                    const std::shared_ptr<const RepositoryNameSet> &,
                    const std::shared_ptr<const QualifiedPackageNameSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Filter candidate PackageID instances.
             */
            std::shared_ptr<const PackageIDSet> ids(
                    const Environment * const,
                    const std::shared_ptr<const PackageIDSet> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            ///\}
    };

    namespace filter
    {
        /**
         * A Filter which accepts all PackageID instances.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE All :
            public Filter
        {
            public:
                All();
        };

        /**
         * A Filter which accepts only PackageID instances which support a given
         * Action subclass.
         *
         * \ingroup g_selections
         */
        template <typename>
        class PALUDIS_VISIBLE SupportsAction :
            public Filter
        {
            public:
                SupportsAction();
        };

        /**
         * A Filter which accepts only PackageID instances which are not masked.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE NotMasked :
            public Filter
        {
            public:
                NotMasked();
        };

        /**
         * A Filter which accepts only PackageID instances that are installed to
         * a particular root.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE InstalledAtRoot :
            public Filter
        {
            public:
                InstalledAtRoot(const FSPath &);
        };

        /**
         * A Filter which accepts only PackageID instances that are installed but
         * not in a particular root.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE InstalledNotAtRoot :
            public Filter
        {
            public:
                InstalledNotAtRoot(const FSPath &);
        };

        /**
         * A Filter which accepts only PackageID instances that are installed to
         * the / fs.
         *
         * \ingroup g_selections
         * \since 0.51
         */
        class PALUDIS_VISIBLE InstalledAtSlash :
            public Filter
        {
            public:
                InstalledAtSlash();
        };

        /**
         * A Filter which accepts only PackageID instances that are installed to
         * an fs other than /.
         *
         * \ingroup g_selections
         * \since 0.51
         */
        class PALUDIS_VISIBLE InstalledAtNotSlash :
            public Filter
        {
            public:
                InstalledAtNotSlash();
        };

        /**
         * A Filter which accepts only PackageID instances that are accepted by
         * two different filters.
         *
         * Used internally by FilteredGenerator.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE And :
            public Filter
        {
            public:
                And(const Filter &, const Filter &);
        };

        /**
         * A Filter which accepts only PackageID instances that have the same
         * slot as the specified PackageID, or, if the specified PackageID has
         * no slot, that have no slot.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE SameSlot :
            public Filter
        {
            public:
                SameSlot(const std::shared_ptr<const PackageID> &);
        };

        /**
         * A Filter which accepts only PackageID instances that have a
         * particular slot.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Slot :
            public Filter
        {
            public:
                Slot(const SlotName &);
        };

        /**
         * A Filter which accepts only PackageID instances that have no slot.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE NoSlot :
            public Filter
        {
            public:
                NoSlot();
        };

        /**
         * A Filter which accepts only PackageID instances that match a
         * particular PackageDepSpec.
         *
         * \ingroup g_selections
         */
        class PALUDIS_VISIBLE Matches :
            public Filter
        {
            public:
                /**
                 * \param spec_id The PackageID the spec comes from. May be null. Used
                 * for [use=] style dependencies.
                 *
                 * \since 0.58 takes spec_id
                 */
                Matches(
                        const PackageDepSpec &,
                        const std::shared_ptr<const PackageID> & from_id,
                        const MatchPackageOptions &);
        };

        /**
         * A Filter which rejects PackageIDs using a function.
         *
         * \ingroup g_selections
         * \since 2.0
         */
        class PALUDIS_VISIBLE ByFunction :
            public Filter
        {
            public:
                ByFunction(
                        const std::function<bool (const std::shared_ptr<const PackageID> &)> &,
                        const std::string &);
        };
    }

    extern template class Pimp<Filter>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<InstallAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<UninstallAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<PretendAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<ConfigAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<FetchAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<InfoAction>;
    extern template class PALUDIS_VISIBLE filter::SupportsAction<PretendFetchAction>;
}

#endif
