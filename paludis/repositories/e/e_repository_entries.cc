/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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

#include "e_repository_entries.hh"
#include "ebuild_entries.hh"
#include <paludis/util/instantiation_policy-impl.hh>
#include <paludis/util/make_shared_ptr.hh>

using namespace paludis;
using namespace paludis::erepository;
template class InstantiationPolicy<ERepositoryEntriesFactory, instantiation_method::SingletonTag>;

ERepositoryEntries::~ERepositoryEntries()
{
}

ERepositoryEntriesFactory::ERepositoryEntriesFactory()
{
}

const std::tr1::shared_ptr<ERepositoryEntries>
ERepositoryEntriesFactory::create(
        const std::string & s,
        const Environment * const env,
        ERepository * const r,
        const ERepositoryParams & p) const
{
    if (s == "ebuild" || s == "exheres")
        return make_shared_ptr(new EbuildEntries(env, r, p));
    throw ConfigurationError("Unrecognised entries type '" + s + "'");
}

