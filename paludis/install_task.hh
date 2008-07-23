/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_INSTALL_TASK_HH
#define PALUDIS_GUARD_PALUDIS_INSTALL_TASK_HH 1

#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator-fwd.hh>
#include <paludis/dep_list.hh>
#include <paludis/dep_list_exceptions.hh>
#include <paludis/tasks_exceptions.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/action-fwd.hh>
#include <paludis/package_database-fwd.hh>

/** \file
 * Declarations for InstallTask.
 *
 * \ingroup g_tasks
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;

#include <paludis/install_task-se.hh>

    /**
     * Task used to install one or more targets.
     *
     * \ingroup g_tasks
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE InstallTask :
        PrivateImplementationPattern<InstallTask>,
        InstantiationPolicy<InstallTask, instantiation_method::NonCopyableTag>
    {
        private:
            void _execute();
            void _build_dep_list();
            void _display_task_list();
            bool _pretend();
            void _main_actions();
            void _one(const DepList::Iterator, const int, const int, const int, const int);
            void _display_failure_summary();

            void _add_target(const std::string &);
            void _add_package_id(const std::tr1::shared_ptr<const PackageID> &);

            std::tr1::shared_ptr<const PackageDepSpec> _unsatisfied(const DepListEntry &) const;
            std::tr1::shared_ptr<const PackageID> _dependent(const DepListEntry &) const;

        protected:
            ///\name Basic operations
            ///\{

            InstallTask(Environment * const env, const DepListOptions & options,
                    std::tr1::shared_ptr<const DestinationsSet> destinations);

            ///\}

            bool already_done(const DepListEntry &) const PALUDIS_ATTRIBUTE((warn_unused_result));
            FetchActionOptions & fetch_action_options() PALUDIS_ATTRIBUTE((warn_unused_result));

        public:
            ///\name Basic operations
            ///\{

            virtual ~InstallTask();

            ///\}

            ///\name DepList and Install behaviour options
            ///\{

            void set_fetch_only(const bool value);
            void set_pretend(const bool value);
            void set_preserve_world(const bool value);
            void set_debug_mode(const InstallActionDebugOption value);
            void set_checks_mode(const InstallActionChecksOption value);
            void set_add_to_world_spec(const std::string &);
            void set_safe_resume(const bool);
            void set_continue_on_failure(const InstallTaskContinueOnFailure);

            ///\}

            ///\name Targets
            ///\{

            void set_targets_from_user_specs(const std::tr1::shared_ptr<const Sequence<std::string> > &);
            void set_targets_from_exact_packages(const std::tr1::shared_ptr<const PackageIDSequence> &);
            void set_targets_from_serialisation(const std::string &, const std::tr1::shared_ptr<const Sequence<std::string> > &);

            void clear();
            void override_target_type(const DepListTargetType);

            struct TargetsConstIteratorTag;
            typedef WrappedForwardIterator<TargetsConstIteratorTag, const std::string> TargetsConstIterator;
            TargetsConstIterator begin_targets() const PALUDIS_ATTRIBUTE((warn_unused_result));
            TargetsConstIterator end_targets() const PALUDIS_ATTRIBUTE((warn_unused_result));

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

            virtual void on_display_failure_summary_pre() = 0;
            virtual void on_display_failure_summary_success(const DepListEntry &) = 0;
            virtual void on_display_failure_summary_failure(const DepListEntry &) = 0;
            virtual void on_display_failure_summary_skipped_unsatisfied(const DepListEntry &, const PackageDepSpec &) = 0;
            virtual void on_display_failure_summary_skipped_dependent(const DepListEntry &, const std::tr1::shared_ptr<const PackageID> &) = 0;
            virtual void on_display_failure_summary_totals(const int, const int, const int, const int, const int) = 0;
            virtual void on_display_failure_summary_post() = 0;

            virtual void on_not_continuing_due_to_errors() = 0;

            virtual void on_pretend_all_pre() = 0;
            virtual void on_pretend_pre(const DepListEntry &) = 0;
            virtual void on_pretend_post(const DepListEntry &) = 0;
            virtual void on_pretend_all_post() = 0;

            virtual void on_fetch_all_pre() = 0;
            virtual void on_fetch_pre(const DepListEntry &, const int x, const int y, const int s, const int f) = 0;
            virtual void on_fetch_post(const DepListEntry &, const int x, const int y, const int s, const int f) = 0;
            virtual void on_fetch_all_post() = 0;

            virtual void on_install_all_pre() = 0;
            virtual void on_install_pre(const DepListEntry &, const int x, const int y, const int s, const int f) = 0;
            virtual void on_install_post(const DepListEntry &, const int x, const int y, const int s, const int f) = 0;
            virtual void on_install_fail(const DepListEntry &, const int x, const int y, const int s, const int f) = 0;
            virtual void on_install_all_post() = 0;

            virtual void on_skip_unsatisfied(const DepListEntry &, const PackageDepSpec &,
                    const int x, const int y, const int s, const int f) = 0;
            virtual void on_skip_dependent(const DepListEntry &, const std::tr1::shared_ptr<const PackageID> &,
                    const int x, const int y, const int s, const int f) = 0;
            virtual void on_skip_already_done(const DepListEntry &, const int, const int, const int, const int) = 0;

            virtual void on_no_clean_needed(const DepListEntry &) = 0;
            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageIDSequence &) = 0;
            virtual void on_clean_pre(const DepListEntry &,
                    const PackageID &,
                    const int x, const int y, const int s, const int f) = 0;
            virtual void on_clean_post(const DepListEntry &,
                    const PackageID &,
                    const int x, const int y, const int s, const int f) = 0;
            virtual void on_clean_fail(const DepListEntry &,
                    const PackageID &,
                    const int x, const int y, const int s, const int f) = 0;
            virtual void on_clean_all_post(const DepListEntry &,
                    const PackageIDSequence &) = 0;

            virtual void on_update_world_pre() = 0;
            virtual void on_update_world(const PackageDepSpec &) = 0;
            virtual void on_update_world(const SetName &) = 0;
            virtual void on_update_world_skip(const PackageDepSpec &, const std::string &) = 0;
            virtual void on_update_world_skip(const SetName &, const std::string &) = 0;
            virtual void on_update_world_post() = 0;
            virtual void on_preserve_world() = 0;

            virtual void on_installed_paludis();

            virtual void on_ambiguous_package_name_error(const AmbiguousPackageNameError &) = 0;
            virtual void on_no_such_package_error(const NoSuchPackageError &) = 0;
            virtual void on_all_masked_error(const AllMaskedError &) = 0;
            virtual void on_additional_requirements_not_met_error(const AdditionalRequirementsNotMetError &) = 0;
            virtual void on_dep_list_error(const DepListError &) = 0;
            virtual void on_had_both_package_and_set_targets_error(const HadBothPackageAndSetTargets &) = 0;
            virtual void on_multiple_set_targets_specified(const MultipleSetTargetsSpecified &) = 0;

            virtual void on_install_action_error(const InstallActionError &) = 0;
            virtual void on_fetch_action_error(const FetchActionError &) = 0;

            ///\}

            ///\name Logic
            ///\{

            virtual void world_update_set(const SetName &);
            virtual void world_update_packages(std::tr1::shared_ptr<const SetSpecTree::ConstItem>);

            ///\}

            /**
             * Run the task.
             */
            virtual void execute();

            /**
             * Fetch our deplist.
             */
            const DepList & dep_list() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our environment.
             */
            Environment * environment() PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our environment.
             */
            const Environment * environment() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Perform a hook. By default, delegates to environment.
             */
            virtual HookResult perform_hook(const Hook &) const
                PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Have we had any resolution failures?
             */
            virtual bool had_resolution_failures() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Have we had any action failures?
             */
            virtual bool had_action_failures() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Serialise the task.
             */
            std::string serialise(const bool undo_failures) const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * The format for serialisation.
             */
            std::string serialised_format() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };
}

#endif
