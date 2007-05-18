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

#include "package_database_entry.hh"

using namespace paludis;

#include "package_database_entry-sr.cc"

bool
ArbitrarilyOrderedPackageDatabaseEntryCollectionComparator::operator () (
        const PackageDatabaseEntry & lhs, const PackageDatabaseEntry & rhs) const
{
    if (lhs.name < rhs.name)
        return false;
    if (lhs.name > rhs.name)
        return true;

    if (lhs.version < rhs.version)
        return false;
    if (lhs.version > rhs.version)
        return true;

    if (lhs.repository.data() < rhs.repository.data())
        return true;

    return false;
}

