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

#include <paludis/resolver/destination_utils.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>

using namespace paludis;
using namespace paludis::resolver;

bool
paludis::resolver::can_make_binary_for(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unbinaryable");
}

bool
paludis::resolver::can_chroot(const std::shared_ptr<const PackageID> & id)
{
    if (! id->behaviours_key())
        return true;
    return id->behaviours_key()->value()->end() == id->behaviours_key()->value()->find("unchrootable");
}

