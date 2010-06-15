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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_WORK_LIST_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_WORK_LIST_HH 1

#include <paludis/resolver/work_list-fwd.hh>
#include <paludis/resolver/work_item-fwd.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/serialise-fwd.hh>
#include <tr1/memory>

namespace paludis
{
    namespace resolver
    {
        template <typename WorkItem_>
        struct WorkListConstIteratorTag;

        template <typename WorkItem_>
        class PALUDIS_VISIBLE WorkList :
            private PrivateImplementationPattern<WorkList<WorkItem_> >
        {
            private:
                using PrivateImplementationPattern<WorkList<WorkItem_> >::_imp;

            public:
                WorkList();
                ~WorkList();

                WorkListIndex append(const std::tr1::shared_ptr<WorkItem_> &);

                int length() const PALUDIS_ATTRIBUTE((warn_unused_result));

                typedef WorkListConstIteratorTag<WorkItem_> ConstIteratorTag;
                typedef WrappedForwardIterator<ConstIteratorTag, const std::tr1::shared_ptr<WorkItem_> > ConstIterator;
                ConstIterator begin() const PALUDIS_ATTRIBUTE((warn_unused_result));
                ConstIterator end() const PALUDIS_ATTRIBUTE((warn_unused_result));

                static const std::tr1::shared_ptr<WorkList<WorkItem_> > deserialise(Deserialisation &) PALUDIS_ATTRIBUTE((warn_unused_result));
                void serialise(Serialiser &) const;
        };

        extern template class WorkList<PretendWorkItem>;
        extern template class WorkList<ExecuteWorkItem>;
    }

    extern template class WrappedForwardIterator<resolver::WorkListConstIteratorTag<resolver::PretendWorkItem>,
           const std::tr1::shared_ptr<resolver::PretendWorkItem> >;
    extern template class WrappedForwardIterator<resolver::WorkListConstIteratorTag<resolver::ExecuteWorkItem>,
           const std::tr1::shared_ptr<resolver::ExecuteWorkItem> >;
}

#endif
