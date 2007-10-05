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

#ifndef PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH
#define PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH 1

#include <paludis/install_task.hh>
#include <paludis/util/set.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <src/output/console_task.hh>
#include <iosfwd>

namespace paludis
{
    class ConsoleInstallTask;

    enum UseDescriptionState
    {
        uds_all,
        uds_changed,
        uds_new
    };

#include <src/output/console_install_task-sr.hh>

    struct UseDescriptionComparator
    {
        bool operator() (const UseDescription &, const UseDescription &) const;
    };

    class PALUDIS_VISIBLE DepTagSummaryDisplayer :
        public ConstVisitor<DepTagVisitorTypes>
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

    class PALUDIS_VISIBLE EntryDepTagDisplayer :
        public ConstVisitor<DepTagVisitorTypes>
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

        private:
            int _counts[last_count];
            tr1::shared_ptr<Set<DepTagEntry> > _all_tags;
            tr1::shared_ptr<Set<UseDescription, UseDescriptionComparator> > _all_use_descriptions;
            tr1::shared_ptr<UseFlagNameSet> _all_expand_prefixes;

            void _add_descriptions(tr1::shared_ptr<const UseFlagNameSet>,
                    const tr1::shared_ptr<const PackageID> &, UseDescriptionState);

        protected:
            ConsoleInstallTask(Environment * const env, const DepListOptions & options,
                    tr1::shared_ptr<const DestinationsSet>);

        public:
            virtual void execute();

            virtual std::string make_x_of_y(const int x, const int y, const int s, const int f);

            virtual void on_build_deplist_pre();
            virtual void on_build_deplist_post();

            virtual void on_build_cleanlist_pre(const DepListEntry &);
            virtual void on_build_cleanlist_post(const DepListEntry &);

            virtual void on_display_merge_list_pre();
            virtual void on_display_merge_list_post();
            virtual void on_not_continuing_due_to_errors();
            virtual void on_display_merge_list_entry(const DepListEntry &);

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

            virtual void on_no_clean_needed(const DepListEntry &);
            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageIDSequence &);
            virtual void on_clean_pre(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);
            virtual void on_clean_post(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);
            virtual void on_clean_fail(const DepListEntry &,
                    const PackageID &, const int x, const int y, const int s, const int f);
            virtual void on_clean_all_post(const DepListEntry &,
                    const PackageIDSequence &);

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
            virtual void on_use_requirements_not_met_error(const UseRequirementsNotMetError &);
            virtual void on_dep_list_error(const DepListError &);
            virtual void on_had_both_package_and_set_targets_error(const HadBothPackageAndSetTargets &);
            virtual void on_multiple_set_targets_specified(const MultipleSetTargetsSpecified &);

            virtual void on_install_action_error(const InstallActionError &);
            virtual void on_fetch_action_error(const FetchActionError &);

            virtual void on_display_failure_summary_pre();
            virtual void on_display_failure_summary_success(const DepListEntry &);
            virtual void on_display_failure_summary_failure(const DepListEntry &);
            virtual void on_display_failure_summary_skipped_unsatisfied(const DepListEntry &, const PackageDepSpec &);
            virtual void on_display_failure_summary_totals(const int, const int, const int, const int);
            virtual void on_display_failure_summary_post();
            virtual void on_display_failure_no_summary();

            ///\name More granular display routines
            ///\{

            virtual void display_clean_all_pre_list_start(const DepListEntry &,
                    const PackageIDSequence &);
            virtual void display_one_clean_all_pre_list_entry(
                    const PackageID &);
            virtual void display_clean_all_pre_list_end(const DepListEntry &,
                    const PackageIDSequence &);

            virtual void display_merge_list_post_counts();
            virtual void display_merge_list_post_tags();

            virtual void display_merge_list_entry_start(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_package_name(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_version(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_repository(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_slot(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_for(const PackageID &, const DisplayMode);
            virtual void display_merge_list_entry_status_and_update_counts(const DepListEntry &,
                    tr1::shared_ptr<const PackageIDSequence>,
                    tr1::shared_ptr<const PackageIDSequence>, const DisplayMode);
            virtual void display_merge_list_entry_use(const DepListEntry &,
                    tr1::shared_ptr<const PackageIDSequence>,
                    tr1::shared_ptr<const PackageIDSequence>, const DisplayMode);
            virtual void display_merge_list_entry_tags(const DepListEntry &, const DisplayMode);
            virtual void display_merge_list_entry_end(const DepListEntry &, const DisplayMode);

            virtual void display_merge_list_entry_mask_reasons(const DepListEntry &);

            virtual void display_tag_summary_start();
            virtual void display_tag_summary_tag_title(const DepTagCategory &);
            virtual void display_tag_summary_tag_pre_text(const DepTagCategory &);
            virtual void display_tag_summary_tag(tr1::shared_ptr<const DepTag>);
            virtual void display_tag_summary_tag_post_text(const DepTagCategory &);
            virtual void display_tag_summary_end();

            virtual void display_merge_list_post_use_descriptions(const std::string &);
            virtual void display_use_summary_start(const std::string &);
            virtual void display_use_summary_flag(const std::string &,
                    Set<UseDescription, UseDescriptionComparator>::ConstIterator,
                    Set<UseDescription, UseDescriptionComparator>::ConstIterator);
            virtual void display_use_summary_end();

            virtual void show_resume_command() const = 0;

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

            tr1::shared_ptr<Set<DepTagEntry> > all_tags()
            {
                return _all_tags;
            }

            tr1::shared_ptr<Set<UseDescription, UseDescriptionComparator> > all_use_descriptions()
            {
                return _all_use_descriptions;
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

            ///\}

            ///\name Makers
            ///\{

            tr1::shared_ptr<DepTagSummaryDisplayer> make_dep_tag_summary_displayer();
            tr1::shared_ptr<EntryDepTagDisplayer> make_entry_dep_tag_displayer();

            ///\}
    };
}

#endif
