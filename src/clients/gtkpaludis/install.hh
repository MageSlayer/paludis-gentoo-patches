/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_SRC_GTKPALUDIS_INSTALL_HH
#define PALUDIS_GUARD_SRC_GTKPALUDIS_INSTALL_HH 1

#include <paludis/tasks/install_task.hh>
#include <paludis/util/private_implementation_pattern.hh>

namespace gtkpaludis
{
    class OurInstallTask :
        protected paludis::InstallTask,
        private paludis::PrivateImplementationPattern<OurInstallTask>
    {
        protected:
            virtual void on_build_deplist_pre();
            virtual void on_build_deplist_post();

            virtual void on_build_cleanlist_pre(const paludis::DepListEntry &);
            virtual void on_build_cleanlist_post(const paludis::DepListEntry &);

            virtual void on_display_merge_list_pre();
            virtual void on_display_merge_list_post();
            virtual void on_display_merge_list_entry(const paludis::DepListEntry &);

            virtual void on_fetch_all_pre();
            virtual void on_fetch_pre(const paludis::DepListEntry &);
            virtual void on_fetch_post(const paludis::DepListEntry &);
            virtual void on_fetch_all_post();

            virtual void on_install_all_pre();
            virtual void on_install_pre(const paludis::DepListEntry &);
            virtual void on_install_post(const paludis::DepListEntry &);
            virtual void on_install_all_post();

            virtual void on_no_clean_needed(const paludis::DepListEntry &);
            virtual void on_clean_all_pre(const paludis::DepListEntry &,
                    const paludis::PackageDatabaseEntryCollection &);
            virtual void on_clean_pre(const paludis::DepListEntry &,
                    const paludis::PackageDatabaseEntry &);
            virtual void on_clean_post(const paludis::DepListEntry &,
                    const paludis::PackageDatabaseEntry &);
            virtual void on_clean_all_post(const paludis::DepListEntry &,
                    const paludis::PackageDatabaseEntryCollection &);

            virtual void on_update_world_pre();
            virtual void on_update_world(const paludis::PackageDepAtom &);
            virtual void on_update_world_skip(const paludis::PackageDepAtom &, const std::string &);
            virtual void on_update_world_post();
            virtual void on_preserve_world();

        public:
            OurInstallTask();
            virtual ~OurInstallTask();

            class Callbacks
            {
                protected:
                    Callbacks();

                public:
                    virtual ~Callbacks();

                    virtual void display_entry(const paludis::DepListEntry & e) = 0;
            };

            void execute(Callbacks * const);

            using InstallTask::clear;
            using InstallTask::add_target;
            using InstallTask::begin_targets;
            using InstallTask::end_targets;
            using InstallTask::set_pretend;
    };
}

#endif
