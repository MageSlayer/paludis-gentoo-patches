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

#ifndef PALUDIS_GUARD_PALUDIS_DEP_LIST_HH
#define PALUDIS_GUARD_PALUDIS_DEP_LIST_HH 1

#include <paludis/dep_atom.hh>
#include <paludis/dep_tag.hh>
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
    /**
     * What type of target are we handling at the top level.
     *
     * \ingroup grpdepresolver
     */
    enum DepListTargetType
    {
        dl_target_package,   ///\< A package, so force reinstalls.
        dl_target_set        ///\< A set, so don't force reinstalls.
    };

    /**
     * When should we reinstall.
     *
     * \ingroup grpdepresolver
     */
    enum DepListReinstallOption
    {
        dl_reinstall_never,             ///\< Never.
        dl_reinstall_always,            ///\< Always.
        dl_reinstall_if_use_changed     ///\< If a USE flag has changed.
    };

    /**
     * When can we fall back to installed?
     *
     * \ingroup grpdepresolver
     */
    enum DepListFallBackOption
    {
        dl_fall_back_as_needed_except_targets,
        dl_fall_back_as_needed,
        dl_fall_back_never
    };

    /**
     * When should we reinstall scm.
     *
     * \ingroup grpdepresolver
     */
    enum DepListReinstallScmOption
    {
        dl_reinstall_scm_never,
        dl_reinstall_scm_always,
        dl_reinstall_scm_daily,
        dl_reinstall_scm_weekly
    };

    /**
     * When should we upgrade.
     *
     * \ingroup grpdepresolver
     */
    enum DepListUpgradeOption
    {
        dl_upgrade_always,          ///\< Always.
        dl_upgrade_as_needed        ///\< Only as needed.
    };

    /**
     * How should we handle a dep class.
     *
     * \ingroup grpdepresolver
     */
    enum DepListDepsOption
    {
        dl_deps_discard,           ///\< Discard it
        dl_deps_pre,               ///\< As a pre dependency
        dl_deps_pre_or_post,       ///\< As a pre dependency with fallback to post
        dl_deps_post,              ///\< As a post dependency
        dl_deps_try_post           ///\< As an optional post dependency
    };

    /**
     * How we handle circular deps.
     *
     * \ingroup grpdepresolver
     */
    enum DepListCircularOption
    {
        dl_circular_error,    ///\< As an error
        dl_circular_discard   ///\< Discard them
    };

    /**
     * State of a DepListEntry.
     *
     * \ingroup grpdepresolver
     */
    enum DepListEntryState
    {
        dle_no_deps,         ///\< Dependencies have yet to be added
        dle_has_pre_deps,    ///\< Predependencies have been added
        dle_has_all_deps     ///\< All dependencies have been added
    };

#include <paludis/dep_list/dep_list-sr.hh>

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
     * Thrown if all versions of a particular atom are masked.
     *
     * \ingroup grpdepresolver
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class AllMaskedError : public DepListError
    {
        private:
            std::string _query;

        public:
            ///\name Basic operations
            ///\{

            AllMaskedError(const std::string & query) throw ();

            virtual ~AllMaskedError() throw ()
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
     * Thrown if all versions of a particular atom are masked,
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

            friend class AddVisitor;
            friend class QueryVisitor;

            void add_in_role(DepAtom::ConstPointer, const std::string & role);
            bool prefer_installed_over_uninstalled(const PackageDatabaseEntry &,
                    const PackageDatabaseEntry &);
            void add_package(const PackageDatabaseEntry &, DepTag::ConstPointer);
            void add_already_installed_package(const PackageDatabaseEntry &, DepTag::ConstPointer);
            void add_predeps(DepAtom::ConstPointer, const DepListDepsOption, const std::string &);
            void add_postdeps(DepAtom::ConstPointer, const DepListDepsOption, const std::string &);
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
            DepListOptions & options;

            /**
             * Add the packages required to resolve an additional dependency
             * atom.
             */
            void add(DepAtom::ConstPointer);

            /**
             * Clear the list.
             */
            void clear();

            /**
             * Is an atom structure already installed?
             */
            bool already_installed(DepAtom::ConstPointer) const;

            /**
             * Is an atom structure already installed (overloaded for raw pointer)?
             */
            bool already_installed(const DepAtom * const) const;

            ///\name Iterate over our dependency list entries.
            ///\{

            typedef libwrapiter::ForwardIterator<DepList, const DepListEntry> Iterator;

            Iterator begin() const;

            Iterator end() const;

            ///\}
    };
}

#endif
