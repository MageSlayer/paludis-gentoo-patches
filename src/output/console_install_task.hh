/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH
#define PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH 1

#include <paludis/install_task.hh>
#include <paludis/util/set.hh>
#include <src/output/console_task.hh>
#include <iosfwd>
#include <map>
#include <list>

namespace paludis
{
    class ConsoleInstallTask;

    class PALUDIS_VISIBLE DepTagSummaryDisplayer
    {
        private:
            ConsoleInstallTask * _task;

        public:
            DepTagSummaryDisplayer(ConsoleInstallTask *) PALUDIS_ATTRIBUTE((nonnull(1)));
            virtual ~DepTagSummaryDisplayer();

            void visit(const GLSADepTag &);
            void visit(const DependencyDepTag &);
            void visit(const GeneralSetDepTag &);
            void visit(const TargetDepTag &);

            ConsoleInstallTask * task()
            {
                return _task;
            }
    };

    class PALUDIS_VISIBLE EntryDepTagDisplayer
    {
        private:
            std::string _text;

        public:
            EntryDepTagDisplayer();
            virtual ~EntryDepTagDisplayer();

            void visit(const GLSADepTag & tag);
            void visit(const DependencyDepTag &);
            void visit(const GeneralSetDepTag & tag);
            void visit(const TargetDepTag &);

            std::string & text()
            {
                return _text;
            }
    };

    class PALUDIS_VISIBLE ConsoleInstallTask :
        public InstallTask,
        public ConsoleTask
    {
        public:
            enum Count
            {
                max_count,
                new_count,
                upgrade_count,
                downgrade_count,
                new_slot_count,
                rebuild_count,
                error_count,
                suggested_count,
                last_count
            };

            enum DisplayMode
            {
                normal_entry,
                unimportant_entry,
                error_entry,
                suggested_entry
            };

            typedef std::map<ChoiceNameWithPrefix, std::list<std::tr1::shared_ptr<const PackageID> > > ChoiceValueDescriptions;
            typedef std::map<std::string, ChoiceValueDescriptions> ChoiceDescriptions;

        private:
            struct CallbackDisplayer;

            int _counts[last_count];
            unsigned long _download_size;
            bool _download_size_overflow;
            std::tr1::shared_ptr<Set<DepTagEntry, DepTagEntryComparator> > _all_tags;
            std::tr1::shared_ptr<Set<FSEntry> > _already_downloaded;
            ChoiceDescriptions _choice_descriptions;

            bool _resolution_finished;

            std::tr1::shared_ptr<CallbackDisplayer> _callback_displayer;
            std::tr1::shared_ptr<NotifierCallbackID> _notifier_callback;

            void _notifier_callback_fn(const NotifierCallbackEvent &);

        protected:
            ConsoleInstallTask(Environment * const env, const DepListOptions & options,
                    const std::tr1::shared_ptr<const DestinationsSet> &);

        public:
            virtual void execute();
            int exit_status() const;

            bool try_to_set_targets_from_user_specs(const std::tr1::shared_ptr<const Sequence<std::string> > &)
                PALUDIS_ATTRIBUTE((warn_unused_result));

            virtual std::string make_x_of_y(const int x, const int y, const int s, const int f);

            virtual void on_build_deplist_pre();
            virtual void on_build_deplist_post();

            virtual void on_display_merge_list_pre();
            virtual void on_display_merge_list_post();
            virtual void on_not_continuing_due_to_errors();
            virtual void on_display_merge_list_entry(const DepListEntry &);

            virtual void on_pretend_all_pre();
            virtual void on_pretend_pre(const DepListEntry &);
            virtual void on_pretend_post(const DepListEntry &);
            virtual void on_pretend_all_post();

            virtual void on_fetch_all_pre();
            virtual void on_fetch_pre(const DepListEntry &, const int x, const int y, const int s, const int f);
            virtual void on_fetch_post(const DepListEntry &, const int x, const int y, const int s, const int f);
            virtual void on_fetch_all_post();

            virtual void on_install_all_pre();
            virtual void on_install_pre(const DepListEntry &, const int x, const int y, const int s, const int f);
            virtual void on_install_post(const DepListEntry &, const int x, const int y, const int s, const int f);
            virtual void on_install_fail(const DepListEntry &, const int x, const int y, const int s, const int f);
            virtual void on_install_all_post();

            virtual void on_skip_unsatisfied(const DepListEntry &, const PackageDepSpec &,
                    const int x, const int y, const int s, const int f);
            virtual void on_skip_dependent(const DepListEntry &, const std::tr1::shared_ptr<const PackageID> &,
                    const int x, const int y, const int s, const int f);
            virtual void on_skip_already_done(const DepListEntry &, const int, const int, const int, const int);

            virtual void on_clean_pre(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);
            virtual void on_clean_post(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);
            virtual void on_clean_fail(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);

            virtual void on_update_world_pre();
            virtual void on_update_world(const PackageDepSpec &);
            virtual void on_update_world(const SetName &);
            virtual void on_update_world_skip(const PackageDepSpec &, const std::string &);
            virtual void on_update_world_skip(const SetName &, const std::string &);
            virtual void on_update_world_post();
            virtual void on_preserve_world();

            virtual void on_ambiguous_package_name_error(const AmbiguousPackageNameError &);
            virtual void on_no_such_package_error(const NoSuchPackageError &);
            virtual void on_all_masked_error(const AllMaskedError &);
            virtual void on_additional_requirements_not_met_error(const AdditionalRequirementsNotMetError &);
            virtual void on_dep_list_error(const DepListError &);
            virtual void on_had_both_package_and_set_targets_error(const HadBothPackageAndSetTargets &);
            virtual void on_multiple_set_targets_specified(const MultipleSetTargetsSpecified &);

            virtual void on_non_fetch_action_error(const std::tr1::shared_ptr<OutputManager> &,
                    const ActionFailedError &);
            virtual void on_fetch_action_error(const std::tr1::shared_ptr<OutputManager> &,
                    const ActionFailedError &,
                    const std::tr1::shared_ptr<const Sequence<FetchActionFailure> > &);

            virtual void on_display_failure_summary_pre();
            virtual void on_display_failure_summary_success(const DepListEntry &);
            virtual void on_display_failure_summary_failure(const DepListEntry &);
            virtual void on_display_failure_summary_skipped_unsatisfied(const DepListEntry &, const PackageDepSpec &);
            virtual void on_display_failure_summary_skipped_dependent(const DepListEntry &, const std::tr1::shared_ptr<const PackageID> &);
            virtual void on_display_failure_summary_totals(const int, const int, const int, const int, const int);
            virtual void on_display_failure_summary_post();

            virtual void on_phase_skip(const std::tr1::shared_ptr<OutputManager> & output_manager, const std::string & phase);
            virtual void on_phase_abort(const std::tr1::shared_ptr<OutputManager> & output_manager, const std::string & phase);
            virtual void on_phase_skip_until(const std::tr1::shared_ptr<OutputManager> & output_manager, const std::string & phase);
            virtual void on_phase_proceed_conditionally(const std::tr1::shared_ptr<OutputManager> & output_manager, const std::string & phase);
            virtual void on_phase_proceed_unconditionally(const std::tr1::shared_ptr<OutputManager> & output_manager, const std::string & phase);

            ///\name More granular display routines
            ///\{

            virtual void display_merge_list_post_counts();
            virtual void display_merge_list_post_tags();
            virtual void display_merge_list_post_use_descriptions();

            virtual void display_merge_list_entry_start(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_package_name(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_repository(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_slot(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_for(const PackageID &, const DisplayMode);
            virtual void display_merge_list_entry_status_and_update_counts(const DepListEntry &,
                    const std::tr1::shared_ptr<const PackageIDSequence> &,
                    const std::tr1::shared_ptr<const PackageIDSequence> &, const DisplayMode);
            virtual void display_merge_list_entry_description(const DepListEntry &,
                    const std::tr1::shared_ptr<const PackageIDSequence> &,
                    const std::tr1::shared_ptr<const PackageIDSequence> &, const DisplayMode);
            virtual void display_merge_list_entry_choices(const DepListEntry &, const DisplayMode,
                    const std::tr1::shared_ptr<const PackageIDSequence> & existing_repo,
                    const std::tr1::shared_ptr<const PackageIDSequence> & existing_slot_repo);
            virtual void display_merge_list_entry_distsize(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_non_package_tags(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_package_tags(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_end(const DepListEntry &, const DisplayMode);

            virtual void display_merge_list_entry_mask_reasons(const DepListEntry &);

            virtual void display_tag_summary_start();
            virtual void display_tag_summary_tag_title(const DepTagCategory &);
            virtual void display_tag_summary_tag_pre_text(const DepTagCategory &);
            virtual void display_tag_summary_tag(const std::tr1::shared_ptr<const DepTag> &);
            virtual void display_tag_summary_tag_post_text(const DepTagCategory &);
            virtual void display_tag_summary_end();

            virtual void display_use_summary_start();
            virtual void display_use_summary_start_choice(const ChoiceDescriptions::const_iterator &);
            virtual void display_use_summary_entry(const ChoiceValueDescriptions::const_iterator &);
            virtual void display_use_summary_end_choice(const ChoiceDescriptions::const_iterator &);
            virtual void display_use_summary_end();

            virtual void show_resume_command() const;
            void show_resume_command(const std::string &) const;
            virtual std::string make_resume_command(const bool undo_failures) const = 0;
            virtual void on_installed_paludis();
            virtual HookResult perform_hook(const Hook &);

            ///\}

            ///\name Data
            ///\{

            template <Count count_>
            int count() const
            {
                return _counts[count_];
            }

            template <Count count_>
            void set_count(const int value)
            {
                _counts[count_] = value;
            }

            long get_download_size() const
            {
                return _download_size;
            }

            std::tr1::shared_ptr<Set<DepTagEntry, DepTagEntryComparator> > all_tags()
            {
                return _all_tags;
            }

            ///\}

            ///\name Options
            ///\{

            virtual bool want_full_install_reasons() const = 0;
            virtual bool want_install_reasons() const = 0;
            virtual bool want_tags_summary() const = 0;

            virtual bool want_use_summary() const = 0;
            virtual bool want_unchanged_use_flags() const = 0;
            virtual bool want_changed_use_flags() const = 0;
            virtual bool want_new_use_flags() const = 0;

            virtual bool want_new_descriptions() const = 0;
            virtual bool want_existing_descriptions() const = 0;

            virtual bool want_compact() const = 0;
            virtual bool want_suggestions() const = 0;

            ///\}

            ///\name Makers
            ///\{

            std::tr1::shared_ptr<DepTagSummaryDisplayer> make_dep_tag_summary_displayer();
            std::tr1::shared_ptr<EntryDepTagDisplayer> make_entry_dep_tag_displayer();

            ///\}
    };
}

#endif
