/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_REPOSITORY_MASK_STORE_HH
#define PALUDIS_GUARD_PALUDIS_REPOSITORIES_E_REPOSITORY_MASK_STORE_HH 1

#include <paludis/repositories/e/mask_info.hh>
#include <paludis/repositories/e/eapi-fwd.hh>
#include <paludis/util/pimp.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/fs_path-fwd.hh>
#include <paludis/util/sequence-fwd.hh>
#include <paludis/environment-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/package_id-fwd.hh>

#include <memory>
#include <functional>

namespace paludis
{
    namespace erepository
    {
        class PALUDIS_VISIBLE RepositoryMaskStore
        {
            private:
                Pimp<RepositoryMaskStore> _imp;

                void _populate();

            public:
                RepositoryMaskStore(
                        const Environment * const,
                        const RepositoryName &,
                        const std::shared_ptr<const FSPathSequence> &,
                        const EAPIForFileFunction &);

                ~RepositoryMaskStore();

                const std::shared_ptr<const MasksInfo> query(const std::shared_ptr<const PackageID> & id) const;
        };
    }

}

#endif
