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

#ifndef PALUDIS_GUARD_PALUDIS_TASKS_INSTALL_TASK_HH
#define PALUDIS_GUARD_PALUDIS_TASKS_INSTALL_TASK_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/dep_list/dep_list.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>

namespace paludis
{
    class Environment;

    /**
     * Task used to install one or more targets.
     *
     * \ingroup grptasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstallTask :
        PrivateImplementationPattern<InstallTask>,
        InstantiationPolicy<InstallTask, instantiation_method::NonCopyableTag>
    {
        protected:
            ///\name Basic operations
            ///\{

            InstallTask(Environment * const env, const DepListOptions & options);

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~InstallTask();

            ///\}

            ///\name DepList and Install behaviour options
            ///\{

            void set_no_config_protect(const bool value);
            void set_fetch_only(const bool value);
            void set_pretend(const bool value);
            void set_preserve_world(const bool value);
            void set_debug_mode(const InstallDebugOption value);
            void set_add_to_world_atom(const std::string &);
            void set_safe_resume(const bool);

            ///\}

            ///\name Targets
            ///\{

            void add_target(const std::string &);
            void clear();
            bool had_set_targets() const;
            bool had_package_targets() const;

            typedef libwrapiter::ForwardIterator<InstallTask, const std::string> TargetsIterator;
            TargetsIterator begin_targets() const;
            TargetsIterator end_targets() const;

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

            virtual void on_not_continuing_due_to_errors() = 0;

            virtual void on_fetch_all_pre() = 0;
            virtual void on_fetch_pre(const DepListEntry &) = 0;
            virtual void on_fetch_post(const DepListEntry &) = 0;
            virtual void on_fetch_all_post() = 0;

            virtual void on_install_all_pre() = 0;
            virtual void on_install_pre(const DepListEntry &) = 0;
            virtual void on_install_post(const DepListEntry &) = 0;
            virtual void on_install_all_post() = 0;

            virtual void on_no_clean_needed(const DepListEntry &) = 0;
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
            virtual void on_update_world(const SetName &) = 0;
            virtual void on_update_world_skip(const PackageDepAtom &, const std::string &) = 0;
            virtual void on_update_world_skip(const SetName &, const std::string &) = 0;
            virtual void on_update_world_post() = 0;
            virtual void on_preserve_world() = 0;

            virtual void on_installed_paludis();

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

            /**
             * Fetch our environment.
             */
            Environment * environment();

            /**
             * Fetch our environment.
             */
            const Environment * environment() const;
    };
}

#endif
