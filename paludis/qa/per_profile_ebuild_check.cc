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

#include "per_profile_ebuild_check.hh"
#include <paludis/qa/deps_visible_check.hh>
#include <paludis/util/virtual_constructor-impl.hh>

using namespace paludis;
using namespace paludis::qa;

template class VirtualConstructor<std::string, std::tr1::shared_ptr<PerProfileEbuildCheck> (*) (),
         virtual_constructor_not_found::ThrowException<NoSuchPerProfileEbuildCheckTypeError> >;

#include <paludis/qa/per_profile_ebuild_check-sr.cc>

PerProfileEbuildCheck::PerProfileEbuildCheck()
{
}

NoSuchPerProfileEbuildCheckTypeError::NoSuchPerProfileEbuildCheckTypeError(const std::string & s) throw () :
    Exception("No such per profile ebuild check type: '" + s + "'")
{
}

PerProfileEbuildCheckMaker::PerProfileEbuildCheckMaker()
{
    register_maker(DepsVisibleCheck::identifier(),
            &MakePerProfileEbuildCheck<DepsVisibleCheck>::make_per_profile_ebuild_check);
}

