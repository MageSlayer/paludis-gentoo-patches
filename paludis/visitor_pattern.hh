/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_VISITOR_PATTERN_HH
#define PALUDIS_GUARD_PALUDIS_VISITOR_PATTERN_HH 1

/** \file
 * Declarations for the Visits, Visitable and VisitableInterface
 * class templates. You must also include visitor_pattern-impl.hh in
 * any implementation files that define subclasses of VisitsComposite
 * or Visitable.
 *
 * \ingroup Utility
 */

namespace paludis
{
    /**
     * Declares that a class can visit a node of type NodeType_.
     *
     * \ingroup Utility
     */
    template <typename NodeType_>
    class Visits
    {
        public:
            /**
             * Interface: visit a node.
             */
            virtual void visit(const NodeType_ * const) = 0;

            /**
             * Destructor.
             */
            virtual ~Visits()
            {
            }
    };

    /**
     * Declare a class visitable by VIVisitorType_. Used in abstract base
     * classes that define a visitable interface but are not themselves
     * visitable. Should be inherited virtually.
     *
     * \ingroup Utility
     */
    template <typename VIVisitorType_>
    class VisitableInterface
    {
        public:
            /**
             * Interface: accept a visitor.
             */
            virtual void accept(VIVisitorType_ * const) const = 0;

            /**
             * Destructor
             */
            virtual ~VisitableInterface()
            {
            }
    };

    /**
     * Declare a class of type VOurType_ visitable by VVisitorType_. Should
     * not use virtual inheritance. Can be used in child classes of an
     * abstract base class that uses VisitableInterface.
     *
     * \ingroup Utility
     */
    template <typename VOurType_, typename VVisitorType_>
    class Visitable : public virtual VisitableInterface<VVisitorType_>
    {
        public:
            /**
             * Accept a visitor.
             */
            virtual void accept(VVisitorType_ * const v) const;

            /**
             * Destructor.
             */
            virtual ~Visitable()
            {
            }
    };
}

#endif
