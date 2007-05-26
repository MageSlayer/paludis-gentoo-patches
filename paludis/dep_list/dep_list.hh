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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HH 1

#include <paludis/dep_spec.hh>
#include <paludis/dep_tag.hh>
#include <paludis/dep_list/options.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>
#include <paludis/version_spec.hh>

#include <iosfwd>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    class VersionMetadata;

#include <paludis/dep_list/dep_list-sr.hh>

    /**
     * Holds a list of dependencies in merge order.
     *
     * \ingroup grpdepresolver
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepList :
        private InstantiationPolicy<DepList, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepList>
    {
        protected:
            class AddVisitor;
            class QueryVisitor;
            class ShowSuggestVisitor;

            friend class AddVisitor;
            friend class QueryVisitor;
            friend class ShowSuggestVisitor;

            /**
             * Find an appropriate destination for a package.
             */
            tr1::shared_ptr<Repository> find_destination(const PackageDatabaseEntry &,
                    tr1::shared_ptr<const DestinationsCollection>);

            /**
             * Add a DepSpec with role context.
             */
            void add_in_role(DependencySpecTree::ConstItem &, const std::string & role,
                    tr1::shared_ptr<const DestinationsCollection>);

            /**
             * Return whether we prefer the first parameter, which is installed,
             * over the second, which isn't.
             */
            bool prefer_installed_over_uninstalled(const PackageDatabaseEntry &,
                    const PackageDatabaseEntry &);

            /**
             * Add a package to the list.
             */
            void add_package(const PackageDatabaseEntry &, tr1::shared_ptr<const DepTag>,
                    tr1::shared_ptr<const DestinationsCollection> destinations);

            /**
             * Add an already installed package to the list.
             */
            void add_already_installed_package(const PackageDatabaseEntry &, tr1::shared_ptr<const DepTag>,
                    tr1::shared_ptr<const DestinationsCollection> destinations);

            /**
             * Add an error package to the list.
             */
            void add_error_package(const PackageDatabaseEntry &, const DepListEntryKind);

            /**
             * Add a suggested package to the list.
             */
            void add_suggested_package(const PackageDatabaseEntry &,
                    tr1::shared_ptr<const DestinationsCollection> destinations);

            /**
             * Add predependencies.
             */
            void add_predeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    tr1::shared_ptr<const DestinationsCollection> destinations);

            /**
             * Add postdependencies.
             */
            void add_postdeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    tr1::shared_ptr<const DestinationsCollection> destinations);

            /**
             * Return whether the specified PackageDatabaseEntry is matched by
             * the top level target.
             */
            bool is_top_level_target(const PackageDatabaseEntry &) const;

            void add_not_top_level(DependencySpecTree::ConstItem &,
                    tr1::shared_ptr<const DestinationsCollection> target_destinations);

        public:
            ///\name Basic operations
            ///\{

            DepList(const Environment * const, const DepListOptions &);

            virtual ~DepList();

            ///\}

            /**
             * Our options.
             */
            tr1::shared_ptr<DepListOptions> options();

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(SetSpecTree::ConstItem &,
                    tr1::shared_ptr<const DestinationsCollection> target_destinations);

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(const PackageDepSpec &,
                    tr1::shared_ptr<const DestinationsCollection> target_destinations);

            /**
             * Clear the list.
             */
            void clear();

            /**
             * Return whether a spec structure is already installed.
             */
            bool already_installed(DependencySpecTree::ConstItem &,
                    tr1::shared_ptr<const DestinationsCollection> target_destinations) const;

            /**
             * Whether we have any errors.
             */
            bool has_errors() const;

            ///\name Iterate over our dependency list entries.
            ///\{

            typedef libwrapiter::ForwardIterator<DepList, const DepListEntry> Iterator;

            Iterator begin() const;

            Iterator end() const;

            ///\}
    };

    /**
     * Extract the destinations from a DepListEntry destinations member.
     *
     * \ingroup grpdepresolver
     */
    tr1::shared_ptr<DestinationsCollection> extract_dep_list_entry_destinations(
            tr1::shared_ptr<SortedCollection<DepListEntryDestination> >) PALUDIS_VISIBLE;
}

#endif
