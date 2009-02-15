/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2009 Ciaran McCreesh
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
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/unmerger.hh>

namespace paludis
{
    class Environment;

    namespace n
    {
        struct config_protect;
        struct config_protect_mask;
        struct contents_file;
        struct environment;
        struct output_manager;
        struct package_id;
        struct root;
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
        NamedValue<n::contents_file, FSEntry> contents_file;
        NamedValue<n::environment, Environment *> environment;
        NamedValue<n::output_manager, std::tr1::shared_ptr<OutputManager> > output_manager;
        NamedValue<n::package_id, std::tr1::shared_ptr<const PackageID> > package_id;
        NamedValue<n::root, FSEntry> root;
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

            VDBUnmergerError(const std::string &) throw ();

            ///\}
    };

    /**
     * Handle unmerging from a VDBRepository.
     *
     * \ingroup grpvdbrepository
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE VDBUnmerger :
        public Unmerger,
        private PrivateImplementationPattern<VDBUnmerger>
    {
        private:
            Implementation<VDBUnmerger> * _imp;
            class FileExtraInfo;
            class SymlinkExtraInfo;
            class MiscExtraInfo;

        protected:
            bool config_protected(const FSEntry &) const;
            std::string make_tidy(const FSEntry &) const;

            void populate_unmerge_set();

            void display(const std::string &) const;

            bool check_file(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            bool check_dir(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            bool check_sym(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;
            bool check_misc(const FSEntry &, const std::tr1::shared_ptr<ExtraInfo> &) const;

        public:
            ///\name Basic operations
            ///\{

            VDBUnmerger(const VDBUnmergerOptions &);
            ~VDBUnmerger();

            ///\}

            virtual Hook extend_hook(const Hook &) const;
    };

}

#endif
