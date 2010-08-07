/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/mask_utils.hh>
#include <paludis/package_id.hh>
#include <paludis/mask.hh>

using namespace paludis;

namespace
{
    struct WeakMask
    {
        bool visit(const UnacceptedMask &) const
        {
            return true;
        }

        bool visit(const UserMask &) const
        {
            return false;
        }

        bool visit(const RepositoryMask &) const
        {
            return true;
        }

        bool visit(const UnsupportedMask &) const
        {
            return false;
        }

        bool visit(const AssociationMask &) const
        {
            return false;
        }
    };
}

bool
paludis::not_strongly_masked(const std::shared_ptr<const PackageID> & id)
{
    for (auto m(id->begin_masks()), m_end(id->end_masks()) ;
            m != m_end ; ++m)
        if (! (*m)->accept_returning<bool>(WeakMask()))
            return false;

    return true;
}

