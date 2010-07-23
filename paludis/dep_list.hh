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
#include <paludis/match_package.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/options.hh>
#include <paludis/version_spec.hh>
#include <functional>
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
    namespace n
    {
        typedef Name<struct associated_entry_name> associated_entry;
        typedef Name<struct blocks_name> blocks;
        typedef Name<struct circular_name> circular;
        typedef Name<struct dependency_tags_name> dependency_tags;
        typedef Name<struct destination_name> destination;
        typedef Name<struct downgrade_name> downgrade;
        typedef Name<struct fall_back_name> fall_back;
        typedef Name<struct generation_name> generation;
        typedef Name<struct handled_name> handled;
        typedef Name<struct installed_deps_post_name> installed_deps_post;
        typedef Name<struct installed_deps_pre_name> installed_deps_pre;
        typedef Name<struct installed_deps_runtime_name> installed_deps_runtime;
        typedef Name<struct kind_name> kind;
        typedef Name<struct match_package_options_name> match_package_options;
        typedef Name<struct new_slots_name> new_slots;
        typedef Name<struct override_masks_name> override_masks;
        typedef Name<struct package_id_name> package_id;
        typedef Name<struct reinstall_name> reinstall;
        typedef Name<struct reinstall_scm_name> reinstall_scm;
        typedef Name<struct state_name> state;
        typedef Name<struct suggested_name> suggested;
        typedef Name<struct tags_name> tags;
        typedef Name<struct target_type_name> target_type;
        typedef Name<struct uninstalled_deps_post_name> uninstalled_deps_post;
        typedef Name<struct uninstalled_deps_pre_name> uninstalled_deps_pre;
        typedef Name<struct uninstalled_deps_runtime_name> uninstalled_deps_runtime;
        typedef Name<struct uninstalled_deps_suggested_name> uninstalled_deps_suggested;
        typedef Name<struct upgrade_name> upgrade;
        typedef Name<struct use_name> use;
    }

    /**
     * A sequence of functions to try, in order, when overriding masks.
     *
     * \ingroup g_dep_list
     */
    typedef Sequence<std::function<bool (const PackageID &, const Mask &)> > DepListOverrideMasksFunctions;

    /**
     * An entry in a DepList.
     *
     * \see DepList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct DepListEntry
    {
        NamedValue<n::associated_entry, const DepListEntry *> associated_entry;
        NamedValue<n::destination, std::shared_ptr<Repository> > destination;
        NamedValue<n::generation, long> generation;
        NamedValue<n::handled, std::shared_ptr<const DepListEntryHandled> > handled;
        NamedValue<n::kind, DepListEntryKind> kind;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
        NamedValue<n::state, DepListEntryState> state;
        NamedValue<n::tags, std::shared_ptr<DepListEntryTags> > tags;
    };

    /**
     * Parameters for a DepList.
     *
     * \see DepList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct PALUDIS_VISIBLE DepListOptions
    {
        DepListOptions();

        NamedValue<n::blocks, DepListBlocksOption> blocks;
        NamedValue<n::circular, DepListCircularOption> circular;
        NamedValue<n::dependency_tags, bool> dependency_tags;
        NamedValue<n::downgrade, DepListDowngradeOption> downgrade;
        NamedValue<n::fall_back, DepListFallBackOption> fall_back;
        NamedValue<n::installed_deps_post, DepListDepsOption> installed_deps_post;
        NamedValue<n::installed_deps_pre, DepListDepsOption> installed_deps_pre;
        NamedValue<n::installed_deps_runtime, DepListDepsOption> installed_deps_runtime;
        NamedValue<n::match_package_options, MatchPackageOptions> match_package_options;
        NamedValue<n::new_slots, DepListNewSlotsOption> new_slots;
        NamedValue<n::override_masks, std::shared_ptr<DepListOverrideMasksFunctions> > override_masks;
        NamedValue<n::reinstall, DepListReinstallOption> reinstall;
        NamedValue<n::reinstall_scm, DepListReinstallScmOption> reinstall_scm;
        NamedValue<n::suggested, DepListSuggestedOption> suggested;
        NamedValue<n::target_type, DepListTargetType> target_type;
        NamedValue<n::uninstalled_deps_post, DepListDepsOption> uninstalled_deps_post;
        NamedValue<n::uninstalled_deps_pre, DepListDepsOption> uninstalled_deps_pre;
        NamedValue<n::uninstalled_deps_runtime, DepListDepsOption> uninstalled_deps_runtime;
        NamedValue<n::uninstalled_deps_suggested, DepListDepsOption> uninstalled_deps_suggested;
        NamedValue<n::upgrade, DepListUpgradeOption> upgrade;
        NamedValue<n::use, DepListUseOption> use;
    };

    /**
     * Holds a list of dependencies in merge order.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE DepList :
        private Pimp<DepList>
    {
        protected:
            class AddVisitor;
            friend class AddVisitor;

            /**
             * Find an appropriate destination for a package.
             */
            std::shared_ptr<Repository> find_destination(const PackageID &,
                    const std::shared_ptr<const DestinationsSet> &);

            /**
             * Add a DepSpec with role context.
             */
            void add_in_role(const bool only_if_not_suggested_label, const DependencySpecTree::BasicNode &, const std::string & role,
                    const std::shared_ptr<const DestinationsSet> &);

            /**
             * Return whether we prefer the first parameter, which is installed,
             * over the second, which isn't.
             */
            bool prefer_installed_over_uninstalled(const PackageID &,
                    const PackageID &);

            /**
             * Add a package to the list.
             */
            void add_package(const std::shared_ptr<const PackageID> &, const std::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an already installed package to the list.
             */
            void add_already_installed_package(const std::shared_ptr<const PackageID> &, const std::shared_ptr<const DepTag> &,
                    const PackageDepSpec &, const std::shared_ptr<const DestinationsSet> & destinations);

            /**
             * Add an error package to the list.
             */
            void add_error_package(const std::shared_ptr<const PackageID> &, const DepListEntryKind, const PackageDepSpec &);

            /**
             * Add predependencies.
             */
            void add_predeps(const DependencySpecTree::BasicNode &, const DepListDepsOption, const std::string &,
                    const std::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Add postdependencies.
             */
            void add_postdeps(const DependencySpecTree::BasicNode &, const DepListDepsOption, const std::string &,
                    const std::shared_ptr<const DestinationsSet> & destinations, const bool only_if_not_suggested_label);

            /**
             * Return whether the specified PackageID is matched by
             * the top level target.
             */
            bool is_top_level_target(const PackageID &) const;

            void add_not_top_level(
                    const bool only_if_not_suggested_label,
                    const DependencySpecTree::BasicNode &,
                    const std::shared_ptr<const DestinationsSet> & target_destinations);

        public:
            ///\name Basic operations
            ///\{

            DepList(const Environment * const, const DepListOptions &);

            virtual ~DepList();

            DepList(const DepList &) = delete;
            DepList & operator= (const DepList &) = delete;

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
            std::shared_ptr<DepListOptions> options();

            /**
             * Our options.
             */
            const std::shared_ptr<const DepListOptions> options() const;

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(const SetSpecTree &,
                    const std::shared_ptr<const DestinationsSet> & target_destinations);

            /**
             * Add the packages required to resolve an additional dependency
             * spec.
             */
            void add(const PackageDepSpec &,
                    const std::shared_ptr<const DestinationsSet> & target_destinations);

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
            bool already_installed(const DependencySpecTree::BasicNode &,
                    const std::shared_ptr<const DestinationsSet> & target_destinations) const;

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
            void add_suggested_package(const std::shared_ptr<const PackageID> &,
                    const PackageDepSpec &, const std::shared_ptr<const DestinationsSet> & destinations);
    };

    extern template class Pimp<DepList>;
    extern template class WrappedForwardIterator<DepList::IteratorTag, DepListEntry>;
    extern template class WrappedForwardIterator<DepList::ConstIteratorTag, const DepListEntry>;
    extern template WrappedForwardIterator<DepList::ConstIteratorTag, const DepListEntry>::WrappedForwardIterator(const DepList::Iterator &);
}

#endif
