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

#include <paludis/repositories/e/do_pretend_fetch_action.hh>
#include <paludis/repositories/e/pretend_fetch_visitor.hh>

#include <paludis/util/stringify.hh>

#include <paludis/action.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace paludis::erepository;

void
paludis::erepository::do_pretend_fetch_action(
        const Environment * const env,
        const ERepository * const repo,
        const std::shared_ptr<const ERepositoryID> & id,
        PretendFetchAction & a)
{
    using namespace std::placeholders;

    Context context("When pretending to fetch ID '" + stringify(*id) + "':");

    if (id->fetches_key())
    {
        PretendFetchVisitor f(env, id, *id->eapi(),
                repo->params().distdir(), a.options.fetch_parts()[fp_unneeded],
                id->fetches_key()->initial_label(), a);
        id->fetches_key()->parse_value()->top()->accept(f);
    }
}

