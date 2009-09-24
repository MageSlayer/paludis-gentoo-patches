/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SIMPLE_VISITOR_HH 1

#include <paludis/util/simple_visitor-fwd.hh>
#include <paludis/util/no_type.hh>

namespace paludis
{
    template <>
    struct DeclareAbstractVisitMethods<TypeListTail>
    {
        void forward_visit(const NoType<0u> &);
    };

    template <typename TypeList_>
    class DeclareAbstractVisitMethods :
        public virtual DeclareAbstractVisitMethods<typename TypeList_::Tail>
    {
        public:
            using DeclareAbstractVisitMethods<typename TypeList_::Tail>::forward_visit;

            virtual void forward_visit(typename TypeList_::Item &) = 0;
    };

    template <typename TypeList_>
    class WrappedVisitorBase :
        public virtual DeclareAbstractVisitMethods<TypeList_>
    {
    };

    template <typename RealClass_>
    class ImplementVisitMethods<RealClass_, TypeListTail>
    {
        public:
            void forward_visit(const NoType<1u> &);
    };

    template <typename RealClass_, typename TypeList_>
    class ImplementVisitMethods :
        public virtual DeclareAbstractVisitMethods<TypeList_>,
        public ImplementVisitMethods<RealClass_, typename TypeList_::Tail>
    {
        public:
            using ImplementVisitMethods<RealClass_, typename TypeList_::Tail>::forward_visit;

            virtual void forward_visit(typename TypeList_::Item & n)
            {
                /* avoid gcc being too clever about noreturn */
                if (this)
                    static_cast<RealClass_ *>(this)->perform_visit(n);
            }
    };

    template <typename TypeList_, typename UnwrappedVisitor_>
    class WrappedVoidResultVisitor :
        public WrappedVisitorBase<TypeList_>,
        public ImplementVisitMethods<WrappedVoidResultVisitor<TypeList_, UnwrappedVisitor_>, TypeList_>
    {
        private:
            UnwrappedVisitor_ & _unwrapped_visitor;

        public:
            WrappedVoidResultVisitor(UnwrappedVisitor_ & v) :
                _unwrapped_visitor(v)
            {
            }

            template <typename C_>
            void perform_visit(C_ & t)
            {
                _unwrapped_visitor.visit(t);
            }
    };

    template <typename TypeList_, typename Result_, typename UnwrappedVisitor_>
    class WrappedNonVoidResultVisitor :
            public WrappedVisitorBase<TypeList_>,
            public ImplementVisitMethods<WrappedNonVoidResultVisitor<TypeList_, Result_, UnwrappedVisitor_>,
                TypeList_>
    {
        private:
            UnwrappedVisitor_ & _unwrapped_visitor;

        public:
            Result_ result;

            WrappedNonVoidResultVisitor(UnwrappedVisitor_ & v, const Result_ & r) :
                _unwrapped_visitor(v),
                result(r)
            {
            }

            template <typename C_>
            void perform_visit(C_ & t)
            {
                result = _unwrapped_visitor.visit(t);
            }
    };

    template <typename BaseClass_, typename VisitableTypeList_>
    class DeclareAbstractAcceptMethods
    {
        private:
            virtual void _real_accept(WrappedVisitorBase<VisitableTypeList_> &) = 0;
            virtual void _real_accept_const(WrappedVisitorBase<typename MakeTypeListConst<VisitableTypeList_>::Type> &) const = 0;

        public:
            typedef VisitableTypeList_ VisitableTypeList;

            template <typename UnwrappedVisitor_>
            void accept(UnwrappedVisitor_ & v)
            {
                WrappedVoidResultVisitor<VisitableTypeList_, UnwrappedVisitor_> vv(v);
                _real_accept(vv);
            }

            template <typename UnwrappedVisitor_>
            void accept(UnwrappedVisitor_ & v) const
            {
                WrappedVoidResultVisitor<typename MakeTypeListConst<VisitableTypeList_>::Type, UnwrappedVisitor_> vv(v);
                _real_accept_const(vv);
            }

            template <typename Result_, typename UnwrappedVisitor_>
            Result_ accept_returning(UnwrappedVisitor_ & v, const Result_ & r = Result_())
            {
                WrappedNonVoidResultVisitor<VisitableTypeList_, Result_, UnwrappedVisitor_> vv(v, r);
                _real_accept(vv);
                return vv.result;
            }

            template <typename Result_, typename UnwrappedVisitor_>
            Result_ accept_returning(const UnwrappedVisitor_ & v, const Result_ & r = Result_())
            {
                WrappedNonVoidResultVisitor<VisitableTypeList_, Result_, const UnwrappedVisitor_> vv(v, r);
                _real_accept(vv);
                return vv.result;
            }

            template <typename Result_, typename UnwrappedVisitor_>
            Result_ accept_returning(UnwrappedVisitor_ & v, const Result_ & r = Result_()) const
            {
                WrappedNonVoidResultVisitor<typename MakeTypeListConst<VisitableTypeList_>::Type, Result_, UnwrappedVisitor_> vv(v, r);
                _real_accept_const(vv);
                return vv.result;
            }

            template <typename Result_, typename UnwrappedVisitor_>
            Result_ accept_returning(const UnwrappedVisitor_ & v, const Result_ & r = Result_()) const
            {
                WrappedNonVoidResultVisitor<typename MakeTypeListConst<VisitableTypeList_>::Type, Result_, const UnwrappedVisitor_> vv(v, r);
                _real_accept_const(vv);
                return vv.result;
            }
    };

    template <typename BaseClass_, typename RealClass_>
    class ImplementAcceptMethods :
        public virtual DeclareAbstractAcceptMethods<BaseClass_, typename BaseClass_::VisitableTypeList>
    {
        private:
            void _real_accept(WrappedVisitorBase<typename BaseClass_::VisitableTypeList> & v)
            {
                v.forward_visit(*static_cast<RealClass_ *>(this));
            };

            void _real_accept_const(WrappedVisitorBase<typename MakeTypeListConst<typename BaseClass_::VisitableTypeList>::Type> & v) const
            {
                v.forward_visit(*static_cast<const RealClass_ *>(this));
            };
    };
}

#endif
