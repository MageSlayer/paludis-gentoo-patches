/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011, 2013 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_MERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_UNPACKAGED_NDBAM_MERGER_HH 1

#include <paludis/fs_merger.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/output_manager-fwd.hh>
#include <paludis/partitioning-fwd.hh>
#include <functional>

namespace paludis
{
    namespace n
    {
        typedef Name<struct name_config_protect> config_protect;
        typedef Name<struct name_config_protect_mask> config_protect_mask;
        typedef Name<struct name_contents_file> contents_file;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_fix_mtimes_before> fix_mtimes_before;
        typedef Name<struct name_fs_merger_options> fs_merger_options;
        typedef Name<struct name_get_new_ids_or_minus_one> get_new_ids_or_minus_one;
        typedef Name<struct name_image> image;
        typedef Name<struct name_install_under> install_under;
        typedef Name<struct name_is_volatile> is_volatile;
        typedef Name<struct name_merged_entries> merged_entries;
        typedef Name<struct name_options> options;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_parts> parts;
        typedef Name<struct name_permit_destination> permit_destination;
        typedef Name<struct name_root> root;
        typedef Name<struct name_should_merge> should_merge;
    }

    struct NDBAMMergerParams
    {
        NamedValue<n::config_protect, std::string> config_protect;
        NamedValue<n::config_protect_mask, std::string> config_protect_mask;
        NamedValue<n::contents_file, FSPath> contents_file;
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
        NamedValue<n::fs_merger_options, FSMergerOptions> fs_merger_options;
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSPath &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSPath> image;
        NamedValue<n::install_under, FSPath> install_under;
        NamedValue<n::is_volatile, std::function<bool (const FSPath &)> > is_volatile;
        NamedValue<n::merged_entries, std::shared_ptr<FSPathSet> > merged_entries;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
        NamedValue<n::parts, std::shared_ptr<const Partitioning> > parts;
        NamedValue<n::permit_destination, PermitDestinationFn> permit_destination;
        NamedValue<n::root, FSPath> root;
        NamedValue<n::should_merge, std::function<bool(const FSPath &)>> should_merge;
    };

    /**
     * Merger subclass for NDBAM.
     *
     * \ingroup g_ndbam
     * \since 0.26
     */
    class PALUDIS_VISIBLE NDBAMMerger :
        public FSMerger
    {
        private:
            void display_override(const std::string &) const override;

            Pimp<NDBAMMerger> _imp;

        public:
            NDBAMMerger(const NDBAMMergerParams &);
            ~NDBAMMerger() override;

            Hook extend_hook(const Hook &) override;

            void record_install_file(const FSPath &, const FSPath &, const std::string &, const FSMergerStatusFlags &) override;
            void record_install_dir(const FSPath &, const FSPath &, const FSMergerStatusFlags &) override;
            void record_install_under_dir(const FSPath &, const FSMergerStatusFlags &) override;
            void record_install_sym(const FSPath &, const FSPath &, const FSMergerStatusFlags &) override;

            void on_error(bool is_check, const std::string &) override;
            void on_warn(bool is_check, const std::string &) override;
            void on_enter_dir(bool is_check, const FSPath) override;

            bool config_protected(const FSPath &, const FSPath &) override;
            std::string make_config_protect_name(const FSPath &, const FSPath &) override;

            void merge() override;
            bool check() override;
    };
}

#endif
