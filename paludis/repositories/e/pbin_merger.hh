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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PBIN_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PBIN_MERGER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/tar_merger.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct environment_name> environment;
        typedef Name<struct environment_file_name> environment_file;
        typedef Name<struct fix_mtimes_before_name> fix_mtimes_before;
        typedef Name<struct image_name> image;
        typedef Name<struct install_under_name> install_under;
        typedef Name<struct merged_entries_name> merged_entries;
        typedef Name<struct options_name> options;
        typedef Name<struct output_manager_name> output_manager;
        typedef Name<struct package_id_name> package_id;
        typedef Name<struct root_name> root;
        typedef Name<struct tar_file_name> tar_file;
    }

    namespace erepository
    {
        struct PbinMergerParams
        {
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::environment_file, FSEntry> environment_file;
            NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
            NamedValue<n::image, FSEntry> image;
            NamedValue<n::install_under, FSEntry> install_under;
            NamedValue<n::merged_entries, std::shared_ptr<FSEntrySet> > merged_entries;
            NamedValue<n::options, MergerOptions> options;
            NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
            NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
            NamedValue<n::root, FSEntry> root;
            NamedValue<n::tar_file, FSEntry> tar_file;
        };

        class PALUDIS_VISIBLE PbinMerger :
            private Pimp<PbinMerger>,
            public TarMerger
        {
            private:
                Pimp<PbinMerger>::ImpPtr & _imp;

            protected:
                virtual void on_error(bool is_check, const std::string &);
                virtual void on_warn(bool is_check, const std::string &);
                virtual void display_override(const std::string &) const;

                virtual void on_enter_dir(bool is_check, const FSEntry);

                virtual void on_done_merge();

            public:
                PbinMerger(const PbinMergerParams &);
                ~PbinMerger();

                virtual Hook extend_hook(const Hook &);

                virtual void merge();
                virtual bool check();
        };
    }
}

#endif
