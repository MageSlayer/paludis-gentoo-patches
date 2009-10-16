/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/fs_entry.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/merger_entry_type.hh>
#include <iosfwd>
#include <sys/stat.h>
#include <sys/types.h>

/** \file
 * Declarations for the Merger class, which can be used by Repository
 * implementations to perform to-filesystem merging.
 *
 * \ingroup g_repository
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    class Environment;
    class Hook;

    namespace n
    {
        struct environment;
        struct get_new_ids_or_minus_one;
        struct image;
        struct install_under;
        struct merged_entries;
        struct no_chown;
        struct options;
        struct root;
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
        NamedValue<n::get_new_ids_or_minus_one, std::tr1::function<std::pair<uid_t, gid_t> (const FSEntry &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSEntry> image;
        NamedValue<n::install_under, FSEntry> install_under;

        /**
         * We record things we merged here.
         *
         * \since 0.41
         */
        NamedValue<n::merged_entries, std::tr1::shared_ptr<FSEntrySet> > merged_entries;

        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::root, FSEntry> root;
    };

    /**
     * Thrown if an error occurs during a Merger operation.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MergerError :
        public Exception
    {
        public:
            ///\name Basic operations
            ///\{

            MergerError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Handles merging an image to a live filesystem.
     *
     * \ingroup g_exceptions
     * \ingroup g_repository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE Merger :
        private PrivateImplementationPattern<Merger>
    {
        private:
            void track_renamed_dir_recursive(const FSEntry &);
            void relabel_dir_recursive(const FSEntry &, const FSEntry &);
            void rewrite_symlink_as_needed(const FSEntry &, const FSEntry &);
            void try_to_copy_xattrs(const FSEntry &, int, MergeStatusFlags &);
            bool symlink_needs_rewriting(const FSEntry &);
            void do_ownership_fixes_recursive(const FSEntry &);

        protected:
            ///\name Basic operations
            ///\{

            Merger(const MergerParams &);

            ///\}

            /**
             * When called, makes check()'s result a failure.
             */
            void make_check_fail();

            /**
             * Allows subclasses to extend hook calls.
             */
            virtual Hook extend_hook(const Hook &);

            /**
             * Determine the entry type of a filesystem entry.
             */
            virtual EntryType entry_type(const FSEntry &);

            /**
             * Handle a directory, recursively.
             */
            virtual void do_dir_recursive(bool is_check, const FSEntry &, const FSEntry &);

            /**
             * Allows subclasses to perform behaviour when entering a directory.
             */
            virtual void on_enter_dir(bool is_check, const FSEntry);

            /**
             * Allows subclasses to perform behaviour when leaving a directory.
             */
            virtual void on_leave_dir(bool is_check, const FSEntry);

            ///\name Track and record merges
            ///\{

            void track_install_file(const FSEntry &, const FSEntry &, const std::string &, const MergeStatusFlags &);
            void track_install_dir(const FSEntry &, const FSEntry &, const MergeStatusFlags &);
            void track_install_under_dir(const FSEntry &, const MergeStatusFlags &);
            void track_install_sym(const FSEntry &, const FSEntry &, const MergeStatusFlags &);

            ///\}

            ///\name Handle filesystem entry things
            ///\{

            virtual void on_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_file_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual MergeStatusFlags install_file(const FSEntry &, const FSEntry &, const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_file(FSEntry);
            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const MergeStatusFlags &) = 0;

            virtual void on_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual MergeStatusFlags install_dir(const FSEntry &, const FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_dir(FSEntry);
            virtual void record_install_dir(const FSEntry &, const FSEntry &, const MergeStatusFlags &) = 0;
            virtual void record_install_under_dir(const FSEntry &, const MergeStatusFlags &) = 0;

            virtual void on_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_nothing(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_file(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_dir(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_sym(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_over_misc(bool is_check, const FSEntry &, const FSEntry &);

            virtual MergeStatusFlags install_sym(const FSEntry &, const FSEntry &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_sym(FSEntry);
            virtual void record_install_sym(const FSEntry &, const FSEntry &, const MergeStatusFlags &) = 0;

            virtual void unlink_misc(FSEntry);
            virtual void on_misc(bool is_check, const FSEntry &, const FSEntry &);

            ///\}

            /**
             * What to do when an error occurs.
             */
            virtual void on_error(bool is_check, const std::string &) = 0;

            /**
             * What to do when a warning occurs.
             */
            virtual void on_warn(bool is_check, const std::string &) = 0;

            virtual void display_override(const std::string &) const = 0;

            ///\name Configuration protection
            ///\{

            virtual bool config_protected(const FSEntry &, const FSEntry &) = 0;
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &) = 0;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~Merger();

            ///\}

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
