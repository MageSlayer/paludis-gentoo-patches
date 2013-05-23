/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2013 Ciaran McCreesh
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

using namespace paludis;
using namespace paludis::resolver;

const std::shared_ptr<const PackageID>
paludis::resolver::get_decided_id_or_null(const std::shared_ptr<const Decision> & d)
{
    return d->make_accept_returning(
            [&] (const ChangesToMakeDecision & decision)    { return decision.origin_id(); },
            [&] (const BreakDecision & decision)            { return decision.existing_id(); },
            [&] (const ExistingNoChangeDecision & decision) { return decision.existing_id(); },
            [&] (const NothingNoChangeDecision &)           { return nullptr; },
            [&] (const RemoveDecision &)                    { return nullptr; },
            [&] (const UnableToMakeDecision &)              { return nullptr; }
        );
}

