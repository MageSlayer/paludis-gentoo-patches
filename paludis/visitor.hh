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

/** \file
 * Declares the Visitor and related classes.
 *
 * \ingroup Visitor
 */

namespace paludis
{
    template <typename NodePtrType_>
    class Visits;

    /**
     * Internal use for Visitor classes.
     *
     * \ingroup Visitor
     */
    namespace visitor_internals
    {
        /**
         * Used as a default parameter when no type is provided. The n_
         * parameter is used to avoid inheriting the same class more than once
         * from a single parent.
         */
        template <unsigned n_>
        struct NoType
        {
        };

        /**
         * Make a pointer to a const.
         */
        template <typename>
        struct MakePointerToConst;

        /**
         * Make a pointer to a const (specialisation for non-const pointers).
         */
        template <typename T_>
        struct MakePointerToConst<T_ *>
        {
            /**
             * Our type.
             */
            typedef const T_ * Type;
        };

        /**
         * Interface: visit a class of NodePtrType_.
         */
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

        /**
         * Interface: don't visit NoType things.
         */
        template <unsigned n_>
        class Visits<const visitor_internals::NoType<n_> * >
        {
            protected:
                ~Visits()
                {
                }
        };

        /**
         * Interface: don't visit NoType things.
         */
        template <unsigned n_>
        class Visits<visitor_internals::NoType<n_> * >
        {
            protected:
                ~Visits()
                {
                }
        };
    }

    /**
     * A class that inherits virtually from VisitableInterface can accept a
     * visitor that is descended from one of the VisitorType_ subclasses.
     */
    template <typename VisitorType_>
    class VisitableInterface
    {
        protected:
            /**
             * Destructor.
             */
            virtual ~VisitableInterface()
            {
            }

        public:
            /**
             * Accept a visitor.
             */
            virtual void accept(typename VisitorType_::Visitor * const) = 0;

            /**
             * Accept a constant visitor.
             */
            virtual void accept(typename VisitorType_::ConstVisitor * const) const = 0;
    };

    /**
     * A class that inherits (non-virtually) from Visitable provides an
     * implementation of VisitableInterface.
     */
    template <typename OurType_, typename VisitorType_>
    class Visitable :
        public virtual VisitableInterface<VisitorType_>
    {
        protected:
            /**
             * Destructor.
             */
            virtual ~Visitable()
            {
            }

        public:
            virtual void accept(typename VisitorType_::Visitor * const v)
            {
                static_cast<visitor_internals::Visits<OurType_ *> *>(v)->visit(
                        static_cast<OurType_ *>(this));
            }

            virtual void accept(typename VisitorType_::ConstVisitor * const v) const
            {
                static_cast<visitor_internals::Visits<const OurType_ *> *>(v)->visit(
                        static_cast<const OurType_ *>(this));
            }
    };

    /**
     * Create the base classes for constant and non-constant visitors to the
     * specified node types.
     */
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
            /**
             * A ConstVisitor descendent visits nodes via a const pointer.
             */
            class ConstVisitor :
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N1_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N2_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N3_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N4_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N5_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N6_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N7_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N8_>::Type>,
                public visitor_internals::Visits<typename visitor_internals::MakePointerToConst<N9_>::Type>
            {
            };

            /**
             * A Visitor descendent visits nodes via a non-const pointer.
             */
            class Visitor :
                public visitor_internals::Visits<N1_>,
                public visitor_internals::Visits<N2_>,
                public visitor_internals::Visits<N3_>,
                public visitor_internals::Visits<N4_>,
                public visitor_internals::Visits<N5_>,
                public visitor_internals::Visits<N6_>,
                public visitor_internals::Visits<N7_>,
                public visitor_internals::Visits<N8_>,
                public visitor_internals::Visits<N9_>
            {
            };
    };
}

#endif
