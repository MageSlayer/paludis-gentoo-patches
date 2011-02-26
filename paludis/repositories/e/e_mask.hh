/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_MASK_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_E_MASK_HH 1

#include <paludis/mask.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/singleton.hh>

namespace paludis
{
    namespace erepository
    {
        class EUnacceptedMaskStore :
            public Singleton<EUnacceptedMaskStore>
        {
            friend class Singleton<EUnacceptedMaskStore>;

            private:
                Pimp<EUnacceptedMaskStore> _imp;

                EUnacceptedMaskStore();
                ~EUnacceptedMaskStore();

            public:
                const std::shared_ptr<const UnacceptedMask> fetch(const char, const std::string &, const std::string &);
        };

        class EUnsupportedMask :
            public UnsupportedMask
        {
            private:
                Pimp<EUnsupportedMask> _imp;

            public:
                EUnsupportedMask(const char, const std::string &, const std::string &);
                ~EUnsupportedMask();

                virtual char key() const;
                virtual const std::string description() const;
                virtual const std::string explanation() const;
        };

        class ERepositoryMask :
            public RepositoryMask
        {
            private:
                Pimp<ERepositoryMask> _imp;

            public:
                ERepositoryMask(const char, const std::string & description,
                        const std::string & comment, const std::string & token, const FSPath &);
                ~ERepositoryMask();

                virtual char key() const;
                const std::string description() const;

                virtual const std::string comment() const;
                virtual const std::string token() const;
                virtual const FSPath mask_file() const;
        };
    }

    extern template class Singleton<erepository::EUnacceptedMaskStore>;
    extern template class Pimp<erepository::EUnacceptedMaskStore>;
}

#endif
