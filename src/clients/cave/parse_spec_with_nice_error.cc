/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Ciaran McCreesh
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

#include "parse_spec_with_nice_error.hh"
#include "exceptions.hh"
#include <paludis/environment.hh>

using namespace paludis;
using namespace cave;

PackageDepSpec
paludis::cave::parse_spec_with_nice_error(
        const std::string & s,
        const Environment * const env,
        const UserPackageDepSpecOptions & o,
        const Filter & f)
{
    try
    {
        return parse_user_package_dep_spec(s, env, o, f);
    }
    catch (const NoSuchPackageError &)
    {
        nothing_matching_error(env, s, f);
    }
}

