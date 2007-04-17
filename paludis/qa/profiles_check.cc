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

#include <paludis/qa/profiles_check.hh>
#include <paludis/qa/categories_check.hh>
#include <paludis/qa/repo_name_check.hh>
#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

template class VirtualConstructor<std::string, std::tr1::shared_ptr<ProfilesCheck> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchProfilesCheckTypeError> >;

ProfilesCheck::ProfilesCheck()
{
}

NoSuchProfilesCheckTypeError::NoSuchProfilesCheckTypeError(const std::string & s) throw () :
    Exception("No such profiles check type: '" + s + "'")
{
}

ProfilesCheckMaker::ProfilesCheckMaker()
{
    register_maker(CategoriesCheck::identifier(), &MakeProfilesCheck<CategoriesCheck>::make_profiles_check);
    register_maker(RepoNameCheck::identifier(), &MakeProfilesCheck<RepoNameCheck>::make_profiles_check);
}


