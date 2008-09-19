/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2008 Ciaran McCreesh
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

#include <paludis/dep_spec-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/dep_tag.hh>
#include <paludis/dep_list_options.hh>
#include <paludis/dep_list-fwd.hh>
#include <paludis/handled_information-fwd.hh>
#include <paludis/name.hh>
#include <paludis/environment.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/options.hh>
#include <paludis/version_spec.hh>
#include <tr1/functional>
#include <iosfwd>

/** \file
 * Declarations for DepList and related classes.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time. Use InstallTask if you need to install things.
 */

namespace paludis
{
    /**
     * A sequence of functions to try, in order, when overriding masks.
     *
     * \ingroup g_dep_list
     */
    typedef Sequence<std::tr1::function<bool (const PackageID &, const Mask &)> > DepListOverrideMasksFunctions;

#include <paludis/dep_list-sr.hh>

    /**
     * Holds a list of dependencies in merge order.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepList :
        private InstantiationPolicy<DepList, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<DepList>
    {
        protected:
            class AddVisitor;
            friend class AddVisitor;

            /**
             * Find an appropriate destination for a package.
             */
            std::tr1::shared_ptr<Repository> find_destination(const PackageID &,
                    const std::tr1::shared_ptr<const DestinationsSet> &);

            /**
             * Add a DepSpec with role context.
             */
            void add_in_role(const bool only_if_not_suggested_label, DependencySpecTree::ConstItem &, const std::string & role,
                    const std::tr1::shared_ptr<const DestinationsSet> &);

            /**
             * Return whether we prefer the first parameter, which is installed,
             * over the second, which isn't.
             */
            bool prefer_installed_over_uninstalled(const PackageID &,
                    const PackageID &);

            /**
             * Add a package to the list.
             */
            void add_package(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an already installed package to the list.
             */
            void add_already_installed_package(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an error package to the list.
             */
            void add_error_package(const std::tr1::shared_ptr<const PackageID> &, const DepListEntryKind,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &);

            /**
             * Add predependencies.
             */
            void add_predeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Add postdependencies.
             */
            void add_postdeps(DependencySpecTree::ConstItem &, const DepListDepsOption, const std::string &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Return whether the specified PackageID is matched by
             * the top level target.
             */
            bool is_top_level_target(const PackageID &) const;

            void add_not_top_level(
                    const bool only_if_not_suggested_label,
                    DependencySpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations,
                    const std::tr1::shared_ptr<ConstTreeSequence<DependencySpecTree, AllDepSpec> > & conditions);

        public:
            ///\name Basic operations
            ///\{

            DepList(const Environment * const, const DepListOptions &);

            virtual ~DepList();

            ///\}

            ///\name Iterate over our dependency list entries.
            ///\{

            struct IteratorTag;
            typedef WrappedForwardIterator<IteratorTag, DepListEntry> Iterator;

            struct ConstIteratorTag;
            typedef WrappedForwardIterator<ConstIteratorTag, const DepListEntry> ConstIterator;

            Iterator begin();
            Iterator end();

            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}

            /**
             * Our options.
             */
            std::tr1::shared_ptr<DepListOptions> options();

            /**
             * Our options.
             */
            const std::tr1::shared_ptr<const DepListOptions> options() const;

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(SetSpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations);

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(const PackageDepSpec &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations);

            /**
             * Manually add a DepListEntry to the list.
             *
             * Does not work well with ordered resolution, and does not do much
             * sanity checking. This is used by InstallTask to implement resume
             * commands and the exec command.
             */
            Iterator push_back(const DepListEntry &);

            /**
             * Clear the list.
             */
            void clear();

            /**
             * Return whether a spec structure is already installed.
             */
            bool already_installed(DependencySpecTree::ConstItem &,
                    const std::tr1::shared_ptr<const DestinationsSet> & target_destinations) const;

            /**
             * Return whether a PackageID has been replaced.
             */
            bool replaced(const PackageID &) const;

            /**
             * Return whether a spec matches an item in the list.
             */
            bool match_on_list(const PackageDepSpec &) const;

            /**
             * Whether we have any errors.
             */
            bool has_errors() const;

            /**
             * Add a suggested package to the list.
             */
            void add_suggested_package(const std::tr1::shared_ptr<const PackageID> &,
                    const PackageDepSpec &, const std::tr1::shared_ptr<DependencySpecTree::ConstItem> &,
                    const std::tr1::shared_ptr<const DestinationsSet> & destinations);
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class PrivateImplementationPattern<DepList>;
#endif
}

#endif
