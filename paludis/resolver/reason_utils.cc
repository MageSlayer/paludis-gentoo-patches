/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011, 2013 Ciaran McCreesh
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

bool
paludis::resolver::is_target(const std::shared_ptr<const Reason> & reason)
{
    return reason->make_accept_returning(
            [&] (const DependencyReason &) { return false; },
            [&] (const DependentReason &)  { return false; },
            [&] (const WasUsedByReason &)  { return false; },
            [&] (const PresetReason &)     { return false; },
            [&] (const ViaBinaryReason &)  { return false; },
            [&] (const TargetReason &)     { return true; },
            [&] (const LikeOtherDestinationTypeReason & r, const Revisit<bool, Reason> & revisit) {
                return revisit(*r.reason_for_other());
            },
            [&] (const SetReason & r, const Revisit<bool, Reason> & revisit) {
                return revisit(*r.reason_for_set());
            }
    );
}

const std::shared_ptr<const PackageID>
paludis::resolver::maybe_from_package_id_from_reason(const std::shared_ptr<const Reason> & reason)
{
    return reason->make_accept_returning(
            [&] (const DependencyReason & r) { return r.from_id(); },
            [&] (const DependentReason &) { return nullptr; },
            [&] (const WasUsedByReason &) { return nullptr; },
            [&] (const PresetReason &) { return nullptr; },
            [&] (const ViaBinaryReason &) { return nullptr; },
            [&] (const TargetReason &) { return nullptr; },
            [&] (const LikeOtherDestinationTypeReason & r, const Revisit<std::shared_ptr<const PackageID>, Reason> & revisit) {
                return revisit(*r.reason_for_other());
            },
            [&] (const SetReason & r, const Revisit<std::shared_ptr<const PackageID>, Reason> & revisit) {
                return revisit(*r.reason_for_set());
            }
            );
}

