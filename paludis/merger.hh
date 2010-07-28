/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/options.hh>
#include <paludis/hook-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/merger_entry_type.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct environment_name> environment;
        typedef Name<struct get_new_ids_or_minus_one_name> get_new_ids_or_minus_one;
        typedef Name<struct image_name> image;
        typedef Name<struct install_under_name> install_under;
        typedef Name<struct merged_entries_name> merged_entries;
        typedef Name<struct no_chown_name> no_chown;
        typedef Name<struct options_name> options;
        typedef Name<struct root_name> root;
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
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSEntry &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSEntry> image;
        NamedValue<n::install_under, FSEntry> install_under;
        NamedValue<n::merged_entries, std::shared_ptr<FSEntrySet> > merged_entries;
        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::root, FSEntry> root;
    };

    class PALUDIS_VISIBLE MergerError :
        public Exception
    {
        public:
            MergerError(const std::string &) throw ();
    };

    class PALUDIS_VISIBLE Merger :
        private Pimp<Merger>
    {
        protected:
            bool symlink_needs_rewriting(const FSEntry &);
            void set_skipped_dir(const bool);
            void do_ownership_fixes_recursive(const FSEntry &);
            bool fixed_ownership_for(const FSEntry &);

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
            virtual void do_dir_recursive(bool is_check, const FSEntry &, const FSEntry &);

            /**
             * Determine the entry type of a filesystem entry.
             */
            virtual EntryType entry_type(const FSEntry &);

            /**
             * Allows subclasses to perform behaviour when entering a directory.
             */
            virtual void on_enter_dir(bool is_check, const FSEntry);

            /**
             * Allows subclasses to perform behaviour when leaving a directory.
             */
            virtual void on_leave_dir(bool is_check, const FSEntry);

            /**
             * What to do when an error occurs.
             */
            virtual void on_error(bool is_check, const std::string &) = 0;

            /**
             * What to do when a warning occurs.
             */
            virtual void on_warn(bool is_check, const std::string &) = 0;

            virtual void display_override(const std::string &) const = 0;

            virtual void on_misc(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym(bool is_check, const FSEntry &, const FSEntry &);

            virtual void on_file_main(bool is_check, const FSEntry &, const FSEntry &) = 0;
            virtual void on_dir_main(bool is_check, const FSEntry &, const FSEntry &) = 0;
            virtual void on_sym_main(bool is_check, const FSEntry &, const FSEntry &) = 0;

            virtual void prepare_install_under() = 0;

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
