/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/version_spec.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/util/set.hh>

using namespace paludis;
using namespace paludis::erepository;

std::shared_ptr<const Set<std::string> >
ERepositoryID::breaks_portage() const
{
    std::shared_ptr<Set<std::string> > why(std::make_shared<Set<std::string>>());
    if (version().has_try_part() || version().has_scm_part() || version().has_local_revision())
        why->insert("version");
    if ((! eapi()->supported()) || eapi()->supported()->breaks_portage())
        why->insert("eapi");
    return why;
}

CannotChangeSCMRevision::CannotChangeSCMRevision(const std::string & id_s, const std::string & revision) throw () :
    Exception("Cannot change SCM revision for '" + id_s + "' to '" + revision + "'")
{
}


