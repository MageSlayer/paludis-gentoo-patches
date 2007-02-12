/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/sr.hh>
#include <paludis/util/fs_entry.hh>

namespace paludis
{
    class Environment;

#include <paludis/repositories/gentoo/vdb_unmerger-sr.hh>

    class VDBUnmergerError :
        public Exception
    {
        public:
            VDBUnmergerError(const std::string &) throw ();
    };

    class VDBUnmerger :
        private PrivateImplementationPattern<VDBUnmerger>
    {
        protected:
            bool config_protected(const FSEntry &);
            std::string make_tidy(const FSEntry &) const;

            template <typename I_>
            void unmerge_non_directories(I_ begin, const I_ end);

            template <typename I_>
            void unmerge_directories(I_ begin, const I_ end);

        public:
            VDBUnmerger(const VDBUnmergerOptions &);
            ~VDBUnmerger();

            void unmerge();
    };

}

#endif
