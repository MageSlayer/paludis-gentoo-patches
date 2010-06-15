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

#include <paludis/resolver/work_lists.hh>
#include <paludis/resolver/work_list.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/make_shared_copy.hh>
#include <paludis/serialise-impl.hh>

using namespace paludis;
using namespace paludis::resolver;

void
WorkLists::serialise(Serialiser & s) const
{
    s.object("WorkLists")
        .member(SerialiserFlags<serialise::might_be_null>(), "execute_work_list", execute_work_list())
        .member(SerialiserFlags<serialise::might_be_null>(), "pretend_work_list", pretend_work_list())
        ;
}

const std::tr1::shared_ptr<WorkLists>
WorkLists::deserialise(Deserialisation & d)
{
    Deserialisator v(d, "WorkLists");

    return make_shared_copy(make_named_values<WorkLists>(
            n::execute_work_list() = v.member<std::tr1::shared_ptr<WorkList<ExecuteWorkItem> > >("execute_work_list"),
            n::pretend_work_list() = v.member<std::tr1::shared_ptr<WorkList<PretendWorkItem> > >("pretend_work_list")
            ));
}
