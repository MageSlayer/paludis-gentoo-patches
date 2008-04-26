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
#include <paludis/util/virtual_constructor-impl.hh>
#include <paludis/util/instantiation_policy-impl.hh>

using namespace paludis;
using namespace paludis::erepository;

template class VirtualConstructor<std::string,
         std::tr1::shared_ptr<ERepositoryEntries> (*) (const Environment * const, ERepository * const,
                 const ERepositoryParams &),
         virtual_constructor_not_found::ThrowException<NoSuchERepositoryEntriesType> >;

template class InstantiationPolicy<ERepositoryEntriesMaker, instantiation_method::SingletonTag>;

ERepositoryEntries::~ERepositoryEntries()
{
}

NoSuchERepositoryEntriesType::NoSuchERepositoryEntriesType(const std::string & format) throw ():
    ConfigurationError("No available maker for E Repository entries type '" + format + "'")
{
}

ERepositoryEntriesMaker::ERepositoryEntriesMaker()
{
    register_maker("ebuild", &EbuildEntries::make_ebuild_entries);
}

