/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/unmerger.hh>

namespace paludis
{
    class Environment;

#include <paludis/repositories/e/vdb_unmerger-sr.hh>

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

            bool check_file(const FSEntry &, tr1::shared_ptr<ExtraInfo>) const;
            bool check_dir(const FSEntry &, tr1::shared_ptr<ExtraInfo>) const;
            bool check_sym(const FSEntry &, tr1::shared_ptr<ExtraInfo>) const;
            bool check_misc(const FSEntry &, tr1::shared_ptr<ExtraInfo>) const;

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
