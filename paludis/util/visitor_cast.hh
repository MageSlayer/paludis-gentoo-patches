/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_CAST_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_VISITOR_CAST_HH 1

#include <paludis/util/visitor.hh>
#include <paludis/util/tr1_type_traits.hh>

namespace paludis
{
    namespace visitor_cast_internals
    {
        template <typename T_, bool b_>
        struct GetResult
        {
            static T_ * get(const void * const)
            {
                return 0;
            }
        };

        template <typename T_>
        struct GetResult<T_, true>
        {
            static T_ * get(const T_ * const t)
            {
                return t;
            }
        };

        template <typename Result_, typename Heirarchy_, typename Item_>
        struct VisitorCastVisitorVisits :
            virtual visitor_internals::Visits<const Item_>
        {
            Result_ * & result;

            VisitorCastVisitorVisits(Result_ * & r) :
                result(r)
            {
            }

            void visit(const Item_ & i)
            {
                result = GetResult<const Result_, tr1::is_same<const Result_, const Item_>::value>::get(&i);
            }
        };

        template <typename Result_, typename Heirarchy_, unsigned u_>
        struct VisitorCastVisitorVisits<Result_, Heirarchy_, const NoType<u_> >
        {
            VisitorCastVisitorVisits(Result_ * &)
            {
            }
        };

        template <typename Result_, typename Heirarchy_, typename Item_>
        struct VisitorCastVisitorVisits<Result_, Heirarchy_, const TreeLeaf<Heirarchy_, Item_> > :
            virtual visitor_internals::Visits<const TreeLeaf<Heirarchy_, Item_> >
        {
            Result_ * & result;

            VisitorCastVisitorVisits(Result_ * & r) :
                result(r)
            {
            }

            void visit_leaf(const Item_ & i)
            {
                result = GetResult<const Result_, tr1::is_same<const Result_, const Item_>::value>::get(&i);
            }
        };

        template <typename Result_, typename Heirarchy_, typename Item_>
        struct VisitorCastVisitorVisits<Result_, Heirarchy_, const ConstTreeSequence<Heirarchy_, Item_> > :
            virtual visitor_internals::Visits<const ConstTreeSequence<Heirarchy_, Item_> >
        {
            Result_ * & result;

            VisitorCastVisitorVisits(Result_ * & r) :
                result(r)
            {
            }

            void visit_sequence(const Item_ & i,
                    typename Heirarchy_::ConstSequenceIterator,
                    typename Heirarchy_::ConstSequenceIterator)
            {
                result = GetResult<const Result_, tr1::is_same<const Result_, const Item_>::value>::get(&i);
            }
        };

        template <typename Result_, typename Heirarchy_, typename Item_>
        struct VisitorCastVisitorVisits<Result_, Heirarchy_, const TreeSequence<Heirarchy_, Item_> > :
            virtual visitor_internals::Visits<const TreeSequence<Heirarchy_, Item_> >
        {
            Result_ * & result;

            VisitorCastVisitorVisits(Result_ * & r) :
                result(r)
            {
            }

            void visit_sequence(const Item_ & i,
                    typename Heirarchy_::SequenceIterator,
                    typename Heirarchy_::SequenceIterator)
            {
                result = GetResult<const Result_, tr1::is_same<const Result_, const Item_>::value>::get(&i);
            }
        };

        template <typename Result_, typename Heirarchy_>
        struct VisitorCastVisitor :
            ConstVisitor<Heirarchy_>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem1>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem2>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem3>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem4>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem5>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem6>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem7>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem8>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem9>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem10>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem11>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem12>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem13>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem14>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem15>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem16>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem17>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem18>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem19>,
            VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem20>
        {
            Result_ * result;

            VisitorCastVisitor() :
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem1>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem2>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem3>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem4>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem5>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem6>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem7>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem8>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem9>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem10>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem11>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem12>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem13>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem14>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem15>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem16>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem17>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem18>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem19>(result),
                VisitorCastVisitorVisits<Result_, Heirarchy_, const typename Heirarchy_::ContainedItem20>(result),
                result(0)
            {
            }
        };
    }

    template <typename Result_, typename Item_>
    Result_ *
    visitor_cast(const Item_ & h)
    {
        visitor_cast_internals::VisitorCastVisitor<Result_, typename Item_::Heirarchy> v;
        h.accept(v);
        return v.result;
    }
}

#endif
