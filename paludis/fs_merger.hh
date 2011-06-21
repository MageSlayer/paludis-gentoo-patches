/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_FS_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_FS_MERGER_HH 1

#include <paludis/fs_merger-fwd.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/options.hh>
#include <paludis/util/named_value.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/merger_entry_type.hh>
#include <paludis/merger.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/hook-fwd.hh>
#include <iosfwd>
#include <sys/stat.h>
#include <sys/types.h>

/** \file
 * Declarations for the FSMerger class, which can be used by Repository
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
     * Parameters for a basic FSMerger.
     *
     * \see Merger
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.30
     * \since 0.51 called FSMergerParams instead of MergerParams
     */
    struct FSMergerParams
    {
        NamedValue<n::environment, Environment *> environment;

        /**
         * Rewrite any mtimes that are before this time to this time, even if
         * preserving mtimes.
         *
         * \since 0.44
         */
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;

        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSPath &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSPath> image;
        NamedValue<n::install_under, FSPath> install_under;

        NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;

        /**
         * We record things we merged here.
         *
         * \since 0.41
         */
        NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;

        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;

        ///\since 0.66
        NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;

        NamedValue<n::root, FSPath> root;
    };

    /**
     * Thrown if an error occurs during an FSMerger operation.
     *
     * \ingroup g_repository
     * \ingroup g_exceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE FSMergerError :
        public MergerError
    {
        public:
            ///\name Basic operations
            ///\{

            FSMergerError(const std::string & msg) throw ();

            ///\}
    };

    /**
     * Handles merging an image to a live filesystem.
     *
     * \ingroup g_exceptions
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.51 called FSMerger instead of Merger
     */
    class PALUDIS_VISIBLE FSMerger :
        public Merger
    {
        private:
            void track_renamed_dir_recursive(const FSPath &);
            void relabel_dir_recursive(const FSPath &, const FSPath &);
            void try_to_copy_xattrs(const FSPath &, int, FSMergerStatusFlags &);

            Pimp<FSMerger> _imp;

        protected:
            ///\name Basic operations
            ///\{

            FSMerger(const FSMergerParams &);

            ///\}

            virtual Hook extend_hook(const Hook &);

            ///\name Track and record merges
            ///\{

            void track_install_file(const FSPath &, const FSPath &, const std::string &, const FSMergerStatusFlags &);
            void track_install_dir(const FSPath &, const FSPath &, const FSMergerStatusFlags &);
            void track_install_under_dir(const FSPath &, const FSMergerStatusFlags &);
            void track_install_sym(const FSPath &, const FSPath &, const FSMergerStatusFlags &);

            ///\}

            ///\name Handle filesystem entry things
            ///\{

            virtual void on_file_main(bool is_check, const FSPath & src, const FSPath & dst);
            virtual void on_file_over_nothing(bool is_check, const FSPath &, const FSPath &);
            virtual void on_file_over_file(bool is_check, const FSPath &, const FSPath &);
            virtual void on_file_over_dir(bool is_check, const FSPath &, const FSPath &);
            virtual void on_file_over_sym(bool is_check, const FSPath &, const FSPath &);
            virtual void on_file_over_misc(bool is_check, const FSPath &, const FSPath &);

            virtual FSMergerStatusFlags install_file(const FSPath &, const FSPath &, const std::string &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_file(FSPath);
            virtual void record_install_file(const FSPath &, const FSPath &, const std::string &, const FSMergerStatusFlags &) = 0;

            virtual void on_dir_main(bool is_check, const FSPath & src, const FSPath & dst);
            virtual void on_dir_over_nothing(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir_over_file(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir_over_dir(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir_over_sym(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir_over_misc(bool is_check, const FSPath &, const FSPath &);

            virtual FSMergerStatusFlags install_dir(const FSPath &, const FSPath &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_dir(FSPath);
            virtual void record_install_dir(const FSPath &, const FSPath &, const FSMergerStatusFlags &) = 0;
            virtual void record_install_under_dir(const FSPath &, const FSMergerStatusFlags &) = 0;

            virtual void on_sym_main(bool is_check, const FSPath & src, const FSPath & dst);
            virtual void on_sym_over_nothing(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym_over_file(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym_over_dir(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym_over_sym(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym_over_misc(bool is_check, const FSPath &, const FSPath &);

            virtual FSMergerStatusFlags install_sym(const FSPath &, const FSPath &) PALUDIS_ATTRIBUTE((warn_unused_result));
            virtual void unlink_sym(FSPath);
            virtual void record_install_sym(const FSPath &, const FSPath &, const FSMergerStatusFlags &) = 0;

            virtual void unlink_misc(FSPath);

            virtual void prepare_install_under();

            virtual FSPath canonicalise_root_path(const FSPath & f);

            virtual void do_dir_recursive(bool is_check, const FSPath &, const FSPath &);

            ///\}

            ///\name Configuration protection
            ///\{

            virtual bool config_protected(const FSPath &, const FSPath &) = 0;
            virtual std::string make_config_protect_name(const FSPath &, const FSPath &) = 0;

            ///\}

        public:
            ///\name Basic operations
            ///\{

            virtual ~FSMerger();

            FSMerger(const FSMerger &) = delete;
            FSMerger & operator= (const FSMerger &) = delete;

            ///\}

            virtual void merge();
    };

}

#endif
