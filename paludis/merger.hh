/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_MERGER_HH 1

#include <paludis/merger-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/options.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/hook-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/merger_entry_type.hh>
#include <paludis/output_manager-fwd.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_fix_mtimes_before> fix_mtimes_before;
        typedef Name<struct name_get_new_ids_or_minus_one> get_new_ids_or_minus_one;
        typedef Name<struct name_image> image;
        typedef Name<struct name_install_under> install_under;
        typedef Name<struct name_maybe_output_manager> maybe_output_manager;
        typedef Name<struct name_merged_entries> merged_entries;
        typedef Name<struct name_no_chown> no_chown;
        typedef Name<struct name_options> options;
        typedef Name<struct name_permit_destination> permit_destination;
        typedef Name<struct name_root> root;
    }

    /**
     * Parameters for a basic Merger.
     *
     * \see Merger
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.30
     */
    struct MergerParams
    {
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSPath &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSPath> image;
        NamedValue<n::install_under, FSPath> install_under;
        NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
        NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;
        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;

        ///\since 0.66
        NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;

        NamedValue<n::root, FSPath> root;
    };

    class PALUDIS_VISIBLE MergerError :
        public Exception
    {
        public:
            MergerError(const std::string &) throw ();
    };

    class PALUDIS_VISIBLE Merger
    {
        private:
            Pimp<Merger> _imp;

        protected:
            bool symlink_needs_rewriting(const FSPath &);
            void rewrite_symlink_as_needed(const FSPath &, const FSPath &);
            void set_skipped_dir(const bool);
            void do_ownership_fixes_recursive(const FSPath &);
            bool fixed_ownership_for(const FSPath &);

            /**
             * Allows subclasses to extend hook calls.
             */
            virtual Hook extend_hook(const Hook &);

            /**
             * When called, makes check()'s result a failure.
             */
            void make_check_fail();

            /**
             * Handle a directory, recursively.
             */
            virtual void do_dir_recursive(bool is_check, const FSPath &, const FSPath &);

            /**
             * Determine the entry type of a filesystem entry.
             */
            virtual EntryType entry_type(const FSPath &);

            /**
             * Allows subclasses to perform behaviour when entering a directory.
             */
            virtual void on_enter_dir(bool is_check, const FSPath);

            /**
             * Allows subclasses to perform behaviour when leaving a directory.
             */
            virtual void on_leave_dir(bool is_check, const FSPath);

            /**
             * Allows subclasses to perform behaviour when everything has been
             * merged, before any cleanup.
             */
            virtual void on_done_merge();

            /**
             * What to do when an error occurs.
             */
            virtual void on_error(bool is_check, const std::string &) = 0;

            /**
             * What to do when a warning occurs.
             */
            virtual void on_warn(bool is_check, const std::string &) = 0;

            virtual void display_override(const std::string &) const = 0;

            virtual void on_misc(bool is_check, const FSPath &, const FSPath &);
            virtual void on_file(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym(bool is_check, const FSPath &, const FSPath &);

            virtual void on_file_main(bool is_check, const FSPath &, const FSPath &) = 0;
            virtual void on_dir_main(bool is_check, const FSPath &, const FSPath &) = 0;
            virtual void on_sym_main(bool is_check, const FSPath &, const FSPath &) = 0;

            virtual void prepare_install_under() = 0;

            virtual FSPath canonicalise_root_path(const FSPath &) = 0;

        public:
            explicit Merger(const MergerParams &);
            ~Merger();

            Merger(const Merger &) = delete;
            Merger & operator= (const Merger &) = delete;

            /**
             * Check a merge, return whether no errors were encountered.
             */
            virtual bool check() PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Perform the merge.
             */
            virtual void merge();
    };
}

#endif
