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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_WORK_LISTS_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_WORK_LISTS_HH 1

#include <paludis/resolver/work_lists-fwd.hh>
#include <paludis/resolver/work_list-fwd.hh>
#include <paludis/resolver/work_item-fwd.hh>
#include <paludis/util/named_value.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace n
    {
        typedef Name<struct execute_work_list_name> execute_work_list;
        typedef Name<struct pretend_work_list_name> pretend_work_list;
    }

    namespace resolver
    {
        struct WorkLists
        {
            NamedValue<n::execute_work_list, std::tr1::shared_ptr<WorkList<ExecuteWorkItem> > > execute_work_list;
            NamedValue<n::pretend_work_list, std::tr1::shared_ptr<WorkList<PretendWorkItem> > > pretend_work_list;

            static const std::tr1::shared_ptr<WorkLists> deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
            void serialise(Serialiser &) const;
        };
    }
}

#endif
