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

namespace paludis
{
    namespace erepository
    {
        class EUnsupportedMask :
            public UnsupportedMask
        {
            private:
                Pimp<EUnsupportedMask> _imp;

            public:
                EUnsupportedMask(const char, const std::string &, const std::string &);
                ~EUnsupportedMask() override;

                char key() const override;
                const std::string description() const override;
                const std::string explanation() const override;
        };

        class ERepositoryMask :
            public RepositoryMask
        {
            private:
                Pimp<ERepositoryMask> _imp;

            public:
                ERepositoryMask(const char, const std::string & description,
                        const std::string & comment, const std::string & token, const FSPath &);
                ~ERepositoryMask() override;

                char key() const override;
                const std::string description() const override;

                const std::string comment() const override;
                const std::string token() const override;
                const FSPath mask_file() const override;
        };

        const std::shared_ptr<const UnacceptedMask> create_e_unaccepted_mask(
                const char c, const std::string & s, const std::string & k);
    }
}

#endif
