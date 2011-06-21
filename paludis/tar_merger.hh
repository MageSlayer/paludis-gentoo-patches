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

#ifndef PALUDIS_GUARD_PALUDIS_TAR_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_TAR_MERGER_HH 1

#include <paludis/tar_merger-fwd.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/merger.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_compression> compression;
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
        typedef Name<struct name_tar_file> tar_file;
    }

    /**
     * Parameters for a basic TarMerger.
     *
     * \see Merger
     * \ingroup g_repository
     * \nosubgrouping
     * \since 0.51
     */
    struct TarMergerParams
    {
        NamedValue<n::compression, TarMergerCompression> compression;
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSPath &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSPath> image;
        NamedValue<n::install_under, FSPath> install_under;
        NamedValue<n::maybe_output_manager, std::shared_ptr<OutputManager> > maybe_output_manager;
        NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;
        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;
        NamedValue<n::root, FSPath> root;
        NamedValue<n::tar_file, FSPath> tar_file;
    };

    class PALUDIS_VISIBLE TarMerger :
        public Merger
    {
        private:
            Pimp<TarMerger> _imp;

        protected:
            virtual FSPath canonicalise_root_path(const FSPath & f);

            virtual void add_file(const FSPath &, const FSPath &);

            virtual void track_install_file(const FSPath &, const FSPath &) = 0;
            virtual void track_install_sym(const FSPath &, const FSPath &) = 0;

        public:
            TarMerger(const TarMergerParams &);
            ~TarMerger();

            virtual void on_file_main(bool is_check, const FSPath &, const FSPath &);
            virtual void on_dir_main(bool is_check, const FSPath &, const FSPath &);
            virtual void on_sym_main(bool is_check, const FSPath &, const FSPath &);

            virtual void prepare_install_under();

            virtual void merge();
    };
}

#endif
