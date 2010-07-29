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

#ifndef PALUDIS_GUARD_PALUDIS_TAR_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_TAR_MERGER_HH 1

#include <paludis/tar_merger-fwd.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/merger.hh>

namespace paludis
{
    namespace n
    {
        typedef Name<struct compression_name> compression;
        typedef Name<struct environment_name> environment;
        typedef Name<struct fix_mtimes_before_name> fix_mtimes_before;
        typedef Name<struct get_new_ids_or_minus_one_name> get_new_ids_or_minus_one;
        typedef Name<struct image_name> image;
        typedef Name<struct install_under_name> install_under;
        typedef Name<struct merged_entries_name> merged_entries;
        typedef Name<struct no_chown_name> no_chown;
        typedef Name<struct options_name> options;
        typedef Name<struct root_name> root;
        typedef Name<struct tar_file_name> tar_file;
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
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSEntry &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSEntry> image;
        NamedValue<n::install_under, FSEntry> install_under;
        NamedValue<n::merged_entries, std::shared_ptr<FSEntrySet> > merged_entries;
        NamedValue<n::no_chown, bool> no_chown;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::root, FSEntry> root;
        NamedValue<n::tar_file, FSEntry> tar_file;
    };

    class PALUDIS_VISIBLE TarMerger :
        private Pimp<TarMerger>,
        public Merger
    {
        private:
            Pimp<TarMerger>::ImpPtr & _imp;

        protected:
            virtual FSEntry canonicalise_root_path(const FSEntry & f);

            virtual void add_file(const FSEntry &, const FSEntry &);

            virtual void track_install_file(const FSEntry &, const FSEntry &) = 0;
            virtual void track_install_sym(const FSEntry &, const FSEntry &) = 0;

        public:
            TarMerger(const TarMergerParams &);
            ~TarMerger();

            virtual void on_file_main(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_dir_main(bool is_check, const FSEntry &, const FSEntry &);
            virtual void on_sym_main(bool is_check, const FSEntry &, const FSEntry &);

            virtual void prepare_install_under();

            virtual void merge();
    };
}

#endif
