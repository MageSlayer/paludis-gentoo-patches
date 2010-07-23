/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Piotr Jaroszyński
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MEMOISED_HASHES_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_MEMOISED_HASHES_HH 1

#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/safe_ifstream-fwd.hh>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE MemoisedHashes :
            public Singleton<MemoisedHashes>,
            private Pimp<MemoisedHashes>
        {
            friend class Singleton<MemoisedHashes>;

            public:
                template <typename H_>
                const std::string get(const FSEntry & file, SafeIFStream & stream) const;

            private:
                MemoisedHashes();
                ~MemoisedHashes();
        };
    }
    extern template class Pimp<erepository::MemoisedHashes>;
    extern template class Singleton<erepository::MemoisedHashes>;
}

#endif
