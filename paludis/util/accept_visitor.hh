/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ACCEPT_VISITOR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ACCEPT_VISITOR_HH 1

#include <paludis/util/attributes.hh>

namespace paludis
{
    /**
     * Used by accept_visitor.
     *
     * \nosubgrouping
     * \ingroup g_visitors
     */
    template <typename Visitor_>
    class PALUDIS_VISIBLE AcceptVisitor
    {
        private:
            Visitor_ & _v;

        public:
            typedef void result_type;

            ///\name Visitor operations
            ///\{

            AcceptVisitor(Visitor_ & v) :
                _v(v)
            {
            }

            template <typename T_>
            void operator() (T_ & t) const
            {
                t.accept(_v);
            }

            ///\}
    };

    /**
     * Used by accept_visitor.
     *
     * \nosubgrouping
     * \ingroup g_visitors
     */
    template <typename Visitor_, typename Returning_>
    class PALUDIS_VISIBLE AcceptVisitorReturning
    {
        private:
            Visitor_ & _v;

        public:
            typedef Returning_ result_type;

            ///\name Visitor operations
            ///\{

            AcceptVisitorReturning(Visitor_ & v) :
                _v(v)
            {
            }

            template <typename T_>
            Returning_ operator() (T_ & t) const
            {
                return t.template accept_returning<Returning_>(_v);
            }

            ///\}
    };

    /**
     * Convenience function for using a visitor with a standard algorithm.
     *
     * \ingroup g_visitors
     */
    template <typename Visitor_>
    AcceptVisitor<Visitor_> PALUDIS_VISIBLE accept_visitor(Visitor_ & v)
    {
        return AcceptVisitor<Visitor_>(v);
    }

    /**
     * Convenience function for using a visitor with a standard algorithm.
     *
     * \ingroup g_visitors
     */
    template <typename Returning_, typename Visitor_>
    AcceptVisitorReturning<Visitor_, Returning_> PALUDIS_VISIBLE accept_visitor_returning(Visitor_ & v)
    {
        return AcceptVisitorReturning<Visitor_, Returning_>(v);
    }
}

#endif
