/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "portage_repository_entries.hh"
#include "ebuild_entries.hh"
#include "ebin_entries.hh"
#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;

template class VirtualConstructor<std::string,
         std::tr1::shared_ptr<PortageRepositoryEntries> (*) (const Environment * const, PortageRepository * const,
                 const PortageRepositoryParams &),
         virtual_constructor_not_found::ThrowException<NoSuchPortageRepositoryEntriesType> >;

PortageRepositoryEntries::~PortageRepositoryEntries()
{
}

NoSuchPortageRepositoryEntriesType::NoSuchPortageRepositoryEntriesType(const std::string & format) throw ():
    ConfigurationError("No available maker for Portage repository entries type '" + format + "'")
{
}

PortageRepositoryEntriesMaker::PortageRepositoryEntriesMaker()
{
    register_maker("ebuild", &EbuildEntries::make_ebuild_entries);
    register_maker("ebin", &EbinEntries::make_ebin_entries);
}

