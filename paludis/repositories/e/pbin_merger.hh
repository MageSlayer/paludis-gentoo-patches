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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PBIN_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_PBIN_MERGER_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/tar_merger.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_environment_file> environment_file;
        typedef Name<struct name_fix_mtimes_before> fix_mtimes_before;
        typedef Name<struct name_image> image;
        typedef Name<struct name_install_under> install_under;
        typedef Name<struct name_merged_entries> merged_entries;
        typedef Name<struct name_options> options;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_permit_destination> permit_destination;
        typedef Name<struct name_root> root;
        typedef Name<struct name_tar_file> tar_file;
    }

    namespace erepository
    {
        struct PbinMergerParams
        {
            NamedValue<n::environment, Environment *> environment;
            NamedValue<n::environment_file, FSPath> environment_file;
            NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
            NamedValue<n::image, FSPath> image;
            NamedValue<n::install_under, FSPath> install_under;
            NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;
            NamedValue<n::options, MergerOptions> options;
            NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
            NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
            NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;
            NamedValue<n::root, FSPath> root;
            NamedValue<n::tar_file, FSPath> tar_file;
        };

        class PALUDIS_VISIBLE PbinMerger :
            public TarMerger
        {
            private:
                Pimp<PbinMerger> _imp;

            protected:
                void on_error(bool is_check, const std::string &) override;
                void on_warn(bool is_check, const std::string &) override;
                void display_override(const std::string &) const override;

                void on_enter_dir(bool is_check, const FSPath) override;

                void on_done_merge() override;

                void track_install_file(const FSPath &, const FSPath &) override;
                void track_install_sym(const FSPath &, const FSPath &) override;

            public:
                PbinMerger(const PbinMergerParams &);
                ~PbinMerger();

                Hook extend_hook(const Hook &) override;

                void merge() override;
                bool check() override;
        };
    }
}

#endif
