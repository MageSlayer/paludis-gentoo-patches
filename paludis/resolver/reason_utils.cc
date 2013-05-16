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

#include <paludis/resolver/reason_utils.hh>
#include <paludis/resolver/reason.hh>

using namespace paludis;
using namespace paludis::resolver;

namespace
{
    struct IsTargetVisitor
    {
        bool visit(const DependencyReason &) const
        {
            return false;
        }

        bool visit(const DependentReason &) const
        {
            return false;
        }

        bool visit(const WasUsedByReason &) const
        {
            return false;
        }

        bool visit(const PresetReason &) const
        {
            return false;
        }

        bool visit(const ViaBinaryReason &) const
        {
            return false;
        }

        bool visit(const TargetReason &) const
        {
            return true;
        }

        bool visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<bool>(*this);
        }

        bool visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<bool>(*this);
        }
    };

    struct FromIDVisitor
    {
        std::shared_ptr<const PackageID> visit(const DependencyReason & r) const
        {
            return r.from_id();
        }

        std::shared_ptr<const PackageID> visit(const DependentReason &) const
        {
            return nullptr;
        }

        std::shared_ptr<const PackageID> visit(const WasUsedByReason &) const
        {
            return nullptr;
        }

        std::shared_ptr<const PackageID> visit(const PresetReason &) const
        {
            return nullptr;
        }

        std::shared_ptr<const PackageID> visit(const ViaBinaryReason &) const
        {
            return nullptr;
        }

        std::shared_ptr<const PackageID> visit(const TargetReason &) const
        {
            return nullptr;
        }

        std::shared_ptr<const PackageID> visit(const LikeOtherDestinationTypeReason & r) const
        {
            return r.reason_for_other()->accept_returning<std::shared_ptr<const PackageID> >(*this);
        }

        std::shared_ptr<const PackageID> visit(const SetReason & r) const
        {
            return r.reason_for_set()->accept_returning<std::shared_ptr<const PackageID> >(*this);
        }
    };
}

bool
paludis::resolver::is_target(const std::shared_ptr<const Reason> & reason)
{
    IsTargetVisitor v;
    return reason->accept_returning<bool>(v);
}

const std::shared_ptr<const PackageID>
paludis::resolver::maybe_from_package_id_from_reason(const std::shared_ptr<const Reason> & reason)
{
    FromIDVisitor v;
    return reason->accept_returning<std::shared_ptr<const PackageID> >(v);
}

