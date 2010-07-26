/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010 Ciaran McCreesh
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
#include <functional>

namespace paludis
{
    namespace n
    {
        typedef Name<struct config_protect_name> config_protect;
        typedef Name<struct config_protect_mask_name> config_protect_mask;
        typedef Name<struct contents_file_name> contents_file;
        typedef Name<struct environment_name> environment;
        typedef Name<struct fix_mtimes_before_name> fix_mtimes_before;
        typedef Name<struct get_new_ids_or_minus_one_name> get_new_ids_or_minus_one;
        typedef Name<struct image_name> image;
        typedef Name<struct install_under_name> install_under;
        typedef Name<struct merged_entries_name> merged_entries;
        typedef Name<struct options_name> options;
        typedef Name<struct output_manager_name> output_manager;
        typedef Name<struct package_id_name> package_id;
        typedef Name<struct root_name> root;
    }

    struct NDBAMMergerParams
    {
        NamedValue<n::config_protect, std::string> config_protect;
        NamedValue<n::config_protect_mask, std::string> config_protect_mask;
        NamedValue<n::contents_file, FSEntry> contents_file;
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::fix_mtimes_before, Timestamp> fix_mtimes_before;
        NamedValue<n::get_new_ids_or_minus_one, std::function<std::pair<uid_t, gid_t> (const FSEntry &)> > get_new_ids_or_minus_one;
        NamedValue<n::image, FSEntry> image;
        NamedValue<n::install_under, FSEntry> install_under;
        NamedValue<n::merged_entries, std::shared_ptr<FSEntrySet> > merged_entries;
        NamedValue<n::options, MergerOptions> options;
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
        NamedValue<n::root, FSEntry> root;
    };

    /**
     * Merger subclass for NDBAM.
     *
     * \ingroup g_ndbam
     * \since 0.26
     */
    class PALUDIS_VISIBLE NDBAMMerger :
        public FSMerger,
        private Pimp<NDBAMMerger>
    {
        private:
            void display_override(const std::string &) const;
            std::string make_arrows(const FSMergerStatusFlags &) const;

            Pimp<NDBAMMerger>::ImpPtr & _imp;

        public:
            NDBAMMerger(const NDBAMMergerParams &);
            ~NDBAMMerger();

            virtual Hook extend_hook(const Hook &);

            virtual void record_install_file(const FSEntry &, const FSEntry &, const std::string &, const FSMergerStatusFlags &);
            virtual void record_install_dir(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &);
            virtual void record_install_under_dir(const FSEntry &, const FSMergerStatusFlags &);
            virtual void record_install_sym(const FSEntry &, const FSEntry &, const FSMergerStatusFlags &);

            virtual void on_error(bool is_check, const std::string &);
            virtual void on_warn(bool is_check, const std::string &);
            virtual void on_enter_dir(bool is_check, const FSEntry);

            virtual bool config_protected(const FSEntry &, const FSEntry &);
            virtual std::string make_config_protect_name(const FSEntry &, const FSEntry &);

            virtual void merge();
            virtual bool check();
    };
}

#endif
