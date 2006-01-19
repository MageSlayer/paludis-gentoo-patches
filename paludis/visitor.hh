/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#ifndef PALUDIS_GUARD_PALUDIS_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_VISITOR_HH 1

namespace paludis
{
    template <typename NodePtrType_>
    class Visits;

    namespace visitor_internals
    {
        template <unsigned n_>
        struct NoType
        {
        };

        template <typename>
        struct MakePointerToConst;

        template <typename T_>
        struct MakePointerToConst<T_ *>
        {
            typedef const T_ * Type;
        };
    }

    template <typename VisitorType_>
    class VisitableInterface
    {
        protected:
            virtual ~VisitableInterface()
            {
            }

        public:
            virtual void accept(typename VisitorType_::Visitor * const) = 0;

            virtual void accept(typename VisitorType_::ConstVisitor * const) const = 0;
    };

    template <typename OurType_, typename VisitorType_>
    class Visitable :
        public virtual VisitableInterface<VisitorType_>
    {
        protected:
            ~Visitable()
            {
            }

        public:
            virtual void accept(typename VisitorType_::Visitor * const v)
            {
                static_cast<Visits<OurType_ *> *>(v)->visit(
                        static_cast<OurType_ *>(this));
            }

            virtual void accept(typename VisitorType_::ConstVisitor * const v) const
            {
                static_cast<Visits<const OurType_ *> *>(v)->visit(
                        static_cast<const OurType_ *>(this));
            }
    };

    template <typename NodePtrType_>
    class Visits
    {
        protected:
            virtual ~Visits()
            {
            }

        public:
            virtual void visit(NodePtrType_ const) = 0;
    };

    template <unsigned n_>
    class Visits<const visitor_internals::NoType<n_> * >
    {
        protected:
            virtual ~Visits()
            {
            }
    };

    template <unsigned n_>
    class Visits<visitor_internals::NoType<n_> * >
    {
        protected:
            virtual ~Visits()
            {
            }
    };

    template <
        typename N1_,
        typename N2_ = visitor_internals::NoType<2> *,
        typename N3_ = visitor_internals::NoType<3> *,
        typename N4_ = visitor_internals::NoType<4> *,
        typename N5_ = visitor_internals::NoType<5> *,
        typename N6_ = visitor_internals::NoType<6> *,
        typename N7_ = visitor_internals::NoType<7> *,
        typename N8_ = visitor_internals::NoType<8> *,
        typename N9_ = visitor_internals::NoType<9> *>
    class VisitorTypes
    {
        private:
            VisitorTypes();


        public:
            class ConstVisitor :
                public Visits<typename visitor_internals::MakePointerToConst<N1_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N2_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N3_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N4_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N5_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N6_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N7_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N8_>::Type>,
                public Visits<typename visitor_internals::MakePointerToConst<N9_>::Type>
            {
            };

            class Visitor :
                public Visits<N1_>,
                public Visits<N2_>,
                public Visits<N3_>,
                public Visits<N4_>,
                public Visits<N5_>,
                public Visits<N6_>,
                public Visits<N7_>,
                public Visits<N8_>,
                public Visits<N9_>
            {
            };
    };
}

#endif
