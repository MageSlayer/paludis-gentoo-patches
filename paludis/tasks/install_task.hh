/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_INSTALL_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_INSTALL_TASK_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_list.hh>

namespace paludis
{
    class Environment;

    class MultipleSetTargetsSpecified :
        public Exception
    {
        public:
            MultipleSetTargetsSpecified() throw ();
    };

    class HadBothPackageAndSetTargets :
        public Exception
    {
        public:
            HadBothPackageAndSetTargets() throw ();
    };

    class InstallTask :
        PrivateImplementationPattern<InstallTask>,
        InstantiationPolicy<InstallTask, instantiation_method::NonCopyableTag>
    {
        protected:
            ///\name Basic operations
            ///\{

            InstallTask(Environment * const env);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~InstallTask();

            ///\}

            ///\name DepList and Install behaviour options
            ///\{

            void set_rdepend_post(const DepListRdependOption value);
            void set_drop_self_circular(const bool value);
            void set_drop_circular(const bool value);
            void set_drop_all(const bool value);
            void set_ignore_installed(const bool value);
            void set_recursive_deps(const bool value);
            void set_max_stack_depth(const int value);
            void set_no_unnecessary_upgrades(const bool value);

            void set_no_config_protect(const bool value);
            void set_fetch_only(const bool value);
            void set_pretend(const bool value);
            void set_preserve_world(const bool value);

            ///\}

            ///\name Add targets
            ///\{

            void add_target(const std::string &);

            ///\}

            ///\name Event callbacks
            ///\{

            virtual void on_build_deplist_pre() = 0;
            virtual void on_build_deplist_post() = 0;

            virtual void on_build_cleanlist_pre(const DepListEntry &) = 0;
            virtual void on_build_cleanlist_post(const DepListEntry &) = 0;

            virtual void on_display_merge_list_pre() = 0;
            virtual void on_display_merge_list_post() = 0;
            virtual void on_display_merge_list_entry(const DepListEntry &) = 0;

            virtual void on_fetch_all_pre() = 0;
            virtual void on_fetch_pre(const DepListEntry &) = 0;
            virtual void on_fetch_post(const DepListEntry &) = 0;
            virtual void on_fetch_all_post() = 0;

            virtual void on_install_all_pre() = 0;
            virtual void on_install_pre(const DepListEntry &) = 0;
            virtual void on_install_post(const DepListEntry &) = 0;
            virtual void on_install_all_post() = 0;

            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageDatabaseEntryCollection &) = 0;
            virtual void on_clean_pre(const DepListEntry &,
                    const PackageDatabaseEntry &) = 0;
            virtual void on_clean_post(const DepListEntry &,
                    const PackageDatabaseEntry &) = 0;
            virtual void on_clean_all_post(const DepListEntry &,
                    const PackageDatabaseEntryCollection &) = 0;

            virtual void on_update_world_pre() = 0;
            virtual void on_update_world(const PackageDepAtom &) = 0;
            virtual void on_update_world_skip(const PackageDepAtom &, const std::string &) = 0;
            virtual void on_update_world_post() = 0;
            virtual void on_preserve_world() = 0;

            ///\}

            /**
             * Run the task.
             */
            void execute();

            /**
             * Fetch our deplist.
             */
            const DepList & dep_list() const;

            /**
             * Fetch our current deplist entry.
             */
            DepList::Iterator current_dep_list_entry() const;
    };
}

#endif
