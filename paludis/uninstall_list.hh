/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UNUSED_LIST_HH
#define PALUDIS_GUARD_PALUDIS_UNUSED_LIST_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/named_value.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/dep_tag-fwd.hh>

/** \file
 * Declarations for UninstallList and related classes.
 *
 * \ingroup g_dep_list
 *
 * \section Examples
 *
 * - None at this time. Use UninstallTask if you need to uninstall things.
 */

namespace paludis
{
#include <paludis/uninstall_list-se.hh>

    namespace n
    {
        struct kind;
        struct package_id;
        struct tags;
        struct with_dependencies_as_errors;
        struct with_dependencies_included;
        struct with_unused_dependencies;
    }

    /**
     * Parameters for an UninstallList.
     *
     * \see UninstallList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct UninstallListOptions
    {
        NamedValue<n::with_dependencies_as_errors, bool> with_dependencies_as_errors;
        NamedValue<n::with_dependencies_included, bool> with_dependencies_included;
        NamedValue<n::with_unused_dependencies, bool> with_unused_dependencies;
    };

    /**
     * An entry in an UninstallList.
     *
     * \see UninstallList
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    struct UninstallListEntry
    {
        NamedValue<n::kind, UninstallListEntryKind> kind;
        NamedValue<n::package_id, std::tr1::shared_ptr<const PackageID> > package_id;
        NamedValue<n::tags, std::tr1::shared_ptr<Set<std::tr1::shared_ptr<DepTag> > > > tags;
    };

    class Environment;

    /**
     * Work out uninstall ordering for packages.
     *
     * \ingroup g_dep_list
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallList :
        private PrivateImplementationPattern<UninstallList>,
        public InstantiationPolicy<UninstallList, instantiation_method::NonCopyableTag>
    {
        private:
            void add_package(const std::tr1::shared_ptr<const PackageID> &, const std::tr1::shared_ptr<DepTag> &,
                    const UninstallListEntryKind k);
            void real_add(const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<DepTag> &, const bool);
            void move_package_to_end(const std::tr1::shared_ptr<const PackageID> &);
            void add_unused_dependencies();
            void add_dependencies(const PackageID &, const bool);

            std::tr1::shared_ptr<const PackageIDSet> collect_depped_upon(
                    const std::tr1::shared_ptr<const PackageIDSet> targets) const;

            std::tr1::shared_ptr<const PackageIDSet> collect_all_installed() const;

            std::tr1::shared_ptr<const PackageIDSet> collect_world() const;

            std::tr1::shared_ptr<const PackageIDSet> collect_used() const;

        public:
            ///\name Basic operations
            ///\{

            UninstallList(const Environment * const, const UninstallListOptions &);
            virtual ~UninstallList();

            ///\}

            /**
             * Our options.
             */
            UninstallListOptions & options;

            /**
             * Add a package, optionally with a reason.
             */
            void add(const std::tr1::shared_ptr<const PackageID> &,
                    const std::tr1::shared_ptr<DepTag> & = std::tr1::shared_ptr<DepTag>());

            /**
             * Add errors for any package on our uninstall list that is required by system.
             */
            void add_errors_for_system();

            /**
             * Add any unused packages that are dependencies of packages to uninstall.
             */
            void add_unused();

            /**
             * Whether we have any errors.
             */
            bool has_errors() const;

            ///\name Iterate over our items to remove
            ///\{

            struct UninstallListTag;
            typedef WrappedForwardIterator<UninstallListTag, const UninstallListEntry> ConstIterator;
            ConstIterator begin() const;
            ConstIterator end() const;

            ///\}
    };

#ifdef PALUDIS_HAVE_EXTERN_TEMPLATE
    extern template class WrappedForwardIterator<UninstallList::UninstallListTag, const UninstallListEntry>;
#endif
}

#endif
