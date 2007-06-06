/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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
#include <paludis/util/sr.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/dep_tag.hh>

#include <libwrapiter/libwrapiter_forward_iterator-fwd.hh>

namespace paludis
{
#include <paludis/dep_list/uninstall_list-sr.hh>

    class Environment;

    /**
     * Work out uninstall ordering for packages.
     *
     * \ingroup grpuninstalllist
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE UninstallList :
        private PrivateImplementationPattern<UninstallList>,
        public InstantiationPolicy<UninstallList, instantiation_method::NonCopyableTag>
    {
        private:
            void add_package(const PackageDatabaseEntry &, tr1::shared_ptr<DepTag>);
            void move_package_to_end(const PackageDatabaseEntry &);
            void add_unused_dependencies();
            void add_dependencies(const PackageDatabaseEntry &);

            tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> collect_depped_upon(
                    const tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> targets) const;

            tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> collect_all_installed() const;

            tr1::shared_ptr<const ArbitrarilyOrderedPackageDatabaseEntryCollection> collect_world() const;

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
            void add(const PackageDatabaseEntry &, tr1::shared_ptr<DepTag> = tr1::shared_ptr<DepTag>());

            /**
             * Add any unused packages that are dependencies of packages to uninstall.
             */
            void add_unused();

            ///\name Iterate over our items to remove
            ///\{

            typedef libwrapiter::ForwardIterator<UninstallList, const UninstallListEntry> Iterator;
            Iterator begin() const;
            Iterator end() const;

            ///\}
    };
}

#endif
