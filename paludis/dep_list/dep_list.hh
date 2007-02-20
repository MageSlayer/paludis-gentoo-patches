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

#include <paludis/dep_atom.hh>
#include <paludis/dep_tag.hh>
#include <paludis/dep_list/options.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/version_spec.hh>

#include <iosfwd>
#include <bitset>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{

#include <paludis/dep_list/dep_list-sr.hh>

    /**
     * Holds a list of dependencies in merge order.
     *
     * \ingroup grpdepresolver
     * \nosubgrouping
     */
    class DepList :
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

            std::tr1::shared_ptr<Repository> find_destination(const PackageDatabaseEntry &,
                    std::tr1::shared_ptr<const DestinationsCollection>);

            void add_in_role(std::tr1::shared_ptr<const DepAtom>, const std::string & role,
                    std::tr1::shared_ptr<const DestinationsCollection>);
            bool prefer_installed_over_uninstalled(const PackageDatabaseEntry &,
                    const PackageDatabaseEntry &);

            void add_package(const PackageDatabaseEntry &, std::tr1::shared_ptr<const DepTag>,
                    std::tr1::shared_ptr<const DestinationsCollection> destinations);
            void add_already_installed_package(const PackageDatabaseEntry &, std::tr1::shared_ptr<const DepTag>,
                    std::tr1::shared_ptr<const DestinationsCollection> destinations);
            void add_error_package(const PackageDatabaseEntry &, const DepListEntryKind);
            void add_suggested_package(const PackageDatabaseEntry &,
                    std::tr1::shared_ptr<const DestinationsCollection> destinations);

            void add_predeps(std::tr1::shared_ptr<const DepAtom>, const DepListDepsOption, const std::string &,
                    std::tr1::shared_ptr<const DestinationsCollection> destinations);
            void add_postdeps(std::tr1::shared_ptr<const DepAtom>, const DepListDepsOption, const std::string &,
                    std::tr1::shared_ptr<const DestinationsCollection> destinations);

            bool is_top_level_target(const PackageDatabaseEntry &) const;

        public:
            ///\name Basic operations
            ///\{

            DepList(const Environment * const, const DepListOptions &);

            virtual ~DepList();

            ///\}

            /**
             * Our options.
             */
            std::tr1::shared_ptr<DepListOptions> options();

            /**
             * Add the packages required to resolve an additional dependency
             * atom.
             */
            void add(std::tr1::shared_ptr<const DepAtom>,
                    std::tr1::shared_ptr<const DestinationsCollection> target_destinations);

            /**
             * Clear the list.
             */
            void clear();

            /**
             * Return whether an atom structure already installed.
             */
            bool already_installed(const DepAtom &,
                    std::tr1::shared_ptr<const DestinationsCollection> target_destinations) const;

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
    std::tr1::shared_ptr<DestinationsCollection> extract_dep_list_entry_destinations(
            std::tr1::shared_ptr<SortedCollection<DepListEntryDestination> >);
}

#endif
