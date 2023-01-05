/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Ciaran McCreesh
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
#include <paludis/metadata_key.hh>
#include <paludis/name.hh>

using namespace paludis;

namespace
{
    struct WeaklyUnaccepted
    {
        bool visit(const MetadataCollectionKey<KeywordNameSet> & k) const
        {
            auto v(k.parse_value());

            if (v->empty())
                return false;

            if (v->end() != v->find(KeywordName("-*")))
                return false;

            return true;
        }

        bool visit(const MetadataKey &) const
        {
            return true;
        }
    };

    struct WeakMask
    {
        const std::shared_ptr<const PackageID> id;

        bool visit(const UnacceptedMask & m) const
        {
            auto k(id->find_metadata(m.unaccepted_key_name()));
            if (k != id->end_metadata())
                return (*k)->accept_returning<bool>(WeaklyUnaccepted{});

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
    };
}

bool
paludis::not_strongly_masked(const std::shared_ptr<const PackageID> & id)
{
    for (const auto & mask : id->masks())
        if (! mask->accept_returning<bool>(WeakMask{id}))
            return false;

    return true;
}

