/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2007 Piotr Jaroszyński
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_VDB_UNMERGER_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_GENTOO_VDB_UNMERGER_HH 1

#include <paludis/repository.hh>
#include <paludis/util/pimp.hh>
#include <paludis/unmerger.hh>

namespace paludis
{
    class Environment;

    namespace n
    {
        typedef Name<struct name_config_protect> config_protect;
        typedef Name<struct name_config_protect_mask> config_protect_mask;
        typedef Name<struct name_contents> contents;
        typedef Name<struct name_environment> environment;
        typedef Name<struct name_ignore> ignore;
        typedef Name<struct name_output_manager> output_manager;
        typedef Name<struct name_package_id> package_id;
        typedef Name<struct name_root> root;
    }

    /**
     * Options for a VDBUnmerger.
     *
     * \see VDBUnmerger
     * \ingroup grpvdbrepository
     * \nosubgrouping
     */
    struct VDBUnmergerOptions
    {
        NamedValue<n::config_protect, std::string> config_protect;
        NamedValue<n::config_protect_mask, std::string> config_protect_mask;
        NamedValue<n::contents, std::shared_ptr<const Contents> > contents;
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::ignore, const std::function<bool (const FSPath &)> > ignore;
        NamedValue<n::output_manager, std::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::shared_ptr<const PackageID> > package_id;
        NamedValue<n::root, FSPath> root;
    };

    /**
     * Thrown if an unmerge from a VDBRepository using VDBUnmerger fails.
     *
     * \ingroup grpvdbrepository
     * \ingroup grpexceptions
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE VDBUnmergerError :
        public UnmergerError
    {
        public:
            ///\name Basic operations
            ///\{

            VDBUnmergerError(const std::string &) noexcept;

            ///\}
    };

    /**
     * Handle unmerging from a VDBRepository.
     *
     * \ingroup grpvdbrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE VDBUnmerger :
        public Unmerger
    {
        private:
            Pimp<VDBUnmerger> _imp;

        protected:
            bool config_protected(const FSPath &) const;
            std::string make_tidy(const FSPath &) const;

            void populate_unmerge_set() override;

            void display(const std::string &) const override;

            bool check_file(const std::shared_ptr<const ContentsEntry> &) const override;
            bool check_dir(const std::shared_ptr<const ContentsEntry> &) const override;
            bool check_sym(const std::shared_ptr<const ContentsEntry> &) const override;
            bool check_misc(const std::shared_ptr<const ContentsEntry> &) const override;

        public:
            ///\name Basic operations
            ///\{

            VDBUnmerger(const VDBUnmergerOptions &);
            ~VDBUnmerger() override;

            ///\}

            Hook extend_hook(const Hook &) const override;
    };

}

#endif
