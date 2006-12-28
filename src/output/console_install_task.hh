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

#ifndef PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH
#define PALUDIS_GUARD_SRC_CONSOLE_INSTALL_TASK_HH 1

#include <paludis/tasks/install_task.hh>
#include <paludis/package_database_entry.hh>
#include <src/output/use_flag_pretty_printer.hh>
#include <iosfwd>

namespace paludis
{
    class ConsoleInstallTask;

    enum UseDescriptionState
    {
        uds_unchanged,
        uds_changed,
        uds_new
    };

#include <src/output/console_install_task-sr.hh>

    class PALUDIS_VISIBLE DepTagSummaryDisplayer :
        public DepTagVisitorTypes::ConstVisitor,
        public InternalCounted<DepTagSummaryDisplayer>
    {
        private:
            ConsoleInstallTask * _task;

        public:
            DepTagSummaryDisplayer(ConsoleInstallTask *) PALUDIS_ATTRIBUTE((nonnull(1)));
            virtual ~DepTagSummaryDisplayer();

            void visit(const GLSADepTag * const tag);
            void visit(const DependencyDepTag * const);
            void visit(const GeneralSetDepTag * const tag);

            ConsoleInstallTask * task()
            {
                return _task;
            }
    };

    class PALUDIS_VISIBLE EntryDepTagDisplayer :
        public DepTagVisitorTypes::ConstVisitor,
        public InternalCounted<EntryDepTagDisplayer>
    {
        private:
            std::string _text;

        public:
            EntryDepTagDisplayer();
            virtual ~EntryDepTagDisplayer();

            void visit(const GLSADepTag * const tag);
            void visit(const DependencyDepTag * const);
            void visit(const GeneralSetDepTag * const tag);

            std::string & text()
            {
                return _text;
            }
    };

    class PALUDIS_VISIBLE ConsoleInstallTask :
        public InstallTask
    {
        public:
            enum Count
            {
                current_count,
                max_count,
                new_count,
                upgrade_count,
                downgrade_count,
                new_slot_count,
                rebuild_count,
                last_count
            };

        private:
            int _counts[last_count];
            SortedCollection<DepTagEntry>::Pointer _all_tags;
            SortedCollection<UseDescription>::Pointer _all_use_descriptions;
            UseFlagNameCollection::Pointer _all_expand_prefixes;

            void _add_descriptions(UseFlagNameCollection::ConstPointer,
                    const PackageDatabaseEntry &, UseDescriptionState);

        protected:
            ConsoleInstallTask(Environment * const env, const DepListOptions & options);

        public:
            virtual void on_build_deplist_pre();
            virtual void on_build_deplist_post();

            virtual void on_build_cleanlist_pre(const DepListEntry &);
            virtual void on_build_cleanlist_post(const DepListEntry &);

            virtual void on_display_merge_list_pre();
            virtual void on_display_merge_list_post();
            virtual void on_display_merge_list_entry(const DepListEntry &);

            virtual void on_fetch_all_pre();
            virtual void on_fetch_pre(const DepListEntry &);
            virtual void on_fetch_post(const DepListEntry &);
            virtual void on_fetch_all_post();

            virtual void on_install_all_pre();
            virtual void on_install_pre(const DepListEntry &);
            virtual void on_install_post(const DepListEntry &);
            virtual void on_install_all_post();

            virtual void on_no_clean_needed(const DepListEntry &);
            virtual void on_clean_all_pre(const DepListEntry &,
                    const PackageDatabaseEntryCollection &);
            virtual void on_clean_pre(const DepListEntry &,
                    const PackageDatabaseEntry &);
            virtual void on_clean_post(const DepListEntry &,
                    const PackageDatabaseEntry &);
            virtual void on_clean_all_post(const DepListEntry &,
                    const PackageDatabaseEntryCollection &);

            virtual void on_update_world_pre();
            virtual void on_update_world(const PackageDepAtom &);
            virtual void on_update_world_skip(const PackageDepAtom &, const std::string &);
            virtual void on_update_world_post();
            virtual void on_preserve_world();

            ///\name Output routines
            ///\{

            virtual std::ostream & output_stream() const;
            virtual std::ostream & output_xterm_stream() const;

            virtual void output_activity_start_message(const std::string &) const;
            virtual void output_activity_end_message() const;
            virtual void output_heading(const std::string &) const;
            virtual void output_xterm_title(const std::string &) const;
            virtual void output_starred_item(const std::string &, const unsigned indent = 0) const;
            virtual void output_starred_item_no_endl(const std::string &) const;
            virtual void output_unstarred_item(const std::string &) const;
            virtual void output_no_endl(const std::string &) const;
            virtual void output_endl() const;

            ///\}

            ///\name Render routines
            ///\{

            virtual std::string render_as_package_name(const std::string &) const;
            virtual std::string render_as_tag(const std::string &) const;
            virtual std::string render_as_unimportant(const std::string &) const;
            virtual std::string render_as_slot_name(const std::string &) const;
            virtual std::string render_as_update_mode(const std::string &) const;
            virtual std::string render_plural(int count, const std::string &, const std::string &) const;

            ///\}

            ///\name More granular display routines
            ///\{

            virtual void display_clean_all_pre_list_start(const DepListEntry &,
                    const PackageDatabaseEntryCollection &);
            virtual void display_one_clean_all_pre_list_entry(
                    const PackageDatabaseEntry &);
            virtual void display_clean_all_pre_list_end(const DepListEntry &,
                    const PackageDatabaseEntryCollection &);

            virtual void display_merge_list_post_counts();
            virtual void display_merge_list_post_tags();

            virtual void display_merge_list_entry_start(const DepListEntry &);
            virtual void display_merge_list_entry_package_name(const DepListEntry &);
            virtual void display_merge_list_entry_version(const DepListEntry &);
            virtual void display_merge_list_entry_repository(const DepListEntry &);
            virtual void display_merge_list_entry_slot(const DepListEntry &);
            virtual void display_merge_list_entry_status_and_update_counts(const DepListEntry &,
                    PackageDatabaseEntryCollection::ConstPointer,
                    PackageDatabaseEntryCollection::ConstPointer);
            virtual void display_merge_list_entry_use(const DepListEntry &,
                    PackageDatabaseEntryCollection::ConstPointer,
                    PackageDatabaseEntryCollection::ConstPointer);
            virtual void display_merge_list_entry_tags(const DepListEntry &);
            virtual void display_merge_list_entry_end(const DepListEntry &);

            virtual void display_tag_summary_start();
            virtual void display_tag_summary_tag_title(const DepTagCategory &);
            virtual void display_tag_summary_tag_pre_text(const DepTagCategory &);
            virtual void display_tag_summary_tag(DepTag::ConstPointer);
            virtual void display_tag_summary_tag_post_text(const DepTagCategory &);
            virtual void display_tag_summary_end();

            virtual void display_merge_list_post_use_descriptions(const std::string &);
            virtual void display_use_summary_start(const std::string &);
            virtual void display_use_summary_flag(const std::string &,
                    SortedCollection<UseDescription>::Iterator,
                    SortedCollection<UseDescription>::Iterator);
            virtual void display_use_summary_end();

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

            SortedCollection<DepTagEntry>::Pointer all_tags()
            {
                return _all_tags;
            }

            SortedCollection<UseDescription>::Pointer all_use_descriptions()
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

            DepTagSummaryDisplayer::Pointer make_dep_tag_summary_displayer();
            EntryDepTagDisplayer::Pointer make_entry_dep_tag_displayer();
            UseFlagPrettyPrinter::Pointer make_use_flag_pretty_printer();

            ///\}
    };
}

#endif
