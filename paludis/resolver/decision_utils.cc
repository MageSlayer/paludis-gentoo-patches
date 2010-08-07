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

#include <paludis/resolver/decision_utils.hh>
#include <paludis/resolver/decision.hh>
#include <paludis/util/make_null_shared_ptr.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    struct IDVisitor
    {
        std::shared_ptr<const PackageID> visit(const ChangesToMakeDecision & decision) const
        {
            return decision.origin_id();
        }

        std::shared_ptr<const PackageID> visit(const BreakDecision & decision) const
        {
            return decision.existing_id();
        }

        std::shared_ptr<const PackageID> visit(const ExistingNoChangeDecision & decision) const
        {
            return decision.existing_id();
        }

        std::shared_ptr<const PackageID> visit(const NothingNoChangeDecision &) const
        {
            return make_null_shared_ptr();
        }

        std::shared_ptr<const PackageID> visit(const RemoveDecision &) const
        {
            return make_null_shared_ptr();
        }

        std::shared_ptr<const PackageID> visit(const UnableToMakeDecision &) const
        {
            return make_null_shared_ptr();
        }
    };
}

const std::shared_ptr<const PackageID>
paludis::resolver::get_decided_id_or_null(const std::shared_ptr<const Decision> & decision)
{
    return decision->accept_returning<std::shared_ptr<const PackageID> >(IDVisitor());
}

