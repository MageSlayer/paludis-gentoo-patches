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

#ifndef PALUDIS_GUARD_PALUDIS_NEST_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_NEST_ATOM_HH 1

#include <paludis/attributes.hh>
#include <paludis/composite_pattern.hh>
#include <paludis/counted_ptr.hh>
#include <paludis/instantiation_policy.hh>
#include <paludis/slot_name.hh>
#include <paludis/use_flag_name.hh>
#include <paludis/visitor.hh>
#include <list>

/** \file
 * Declarations for the NestAtom classes.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    class NestAtom;
    class CompositeNestAtom;
    class TextNestAtom;
    class AllNestAtom;
    class UseNestAtom;

    /**
     * Visitor types for a visitor that can visit a NestAtom heirarchy.
     */
    typedef VisitorTypes<TextNestAtom *, AllNestAtom *, UseNestAtom *> NestAtomVisitorTypes;

    /**
     * Base class for a dependency atom.
     *
     * \ingroup DepResolver
     */
    class NestAtom :
        public virtual VisitableInterface<NestAtomVisitorTypes>,
        public virtual Composite<NestAtom, CompositeNestAtom>,
        private InstantiationPolicy<NestAtom, instantiation_method::NonCopyableTag>,
        public InternalCounted<NestAtom>
    {
        protected:
            NestAtom();

        public:
            virtual ~NestAtom();

            /**
             * Return us as a UseNestAtom, or 0 if we are not a
             * UseNestAtom.
             */
            virtual const UseNestAtom * as_use_nest_atom() const PALUDIS_ATTRIBUTE((pure));
    };

    /**
     * Class for nest atoms that have a number of child nest
     * atoms.
     *
     * \ingroup DepResolver
     */
    class CompositeNestAtom :
        public NestAtom,
        public virtual Composite<NestAtom, CompositeNestAtom>
    {
        private:
            std::list<NestAtom::ConstPointer> _children;

        protected:
            /**
             * Constructor.
             */
            CompositeNestAtom();

        public:
            /**
             * Append a child to our collection.
             */
            virtual void add_child(NestAtom::ConstPointer);

            /**
             * Iterator for iterating over our children.
             */
            typedef std::list<NestAtom::ConstPointer>::const_iterator Iterator;

            /**
             * Iterator to the start of our children.
             */
            Iterator begin() const
            {
                return _children.begin();
            }

            /**
             * Iterator to past the end of our children.
             */
            Iterator end() const
            {
                return _children.end();
            }

            typedef CountedPtr<CompositeNestAtom, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const CompositeNestAtom, count_policy::InternalCountTag> ConstPointer;
    };

    /**
     * Represents a ( first second third ) or top level group of nest
     * atoms.
     *
     * \ingroup DepResolver
     */
    class AllNestAtom :
        public CompositeNestAtom,
        public Visitable<AllNestAtom, NestAtomVisitorTypes>
    {
        public:
            /**
             * Constructor.
             */
            AllNestAtom();
    };

    /**
     * Represents a use? ( ) dependency atom.
     *
     * \ingroup DepResolver
     */
    class UseNestAtom :
        public CompositeNestAtom,
        public Visitable<UseNestAtom, NestAtomVisitorTypes>
    {
        private:
            const UseFlagName _flag;
            const bool _inverse;

        public:
            /**
             * Constructor.
             */
            UseNestAtom(const UseFlagName &, bool);

            /**
             * Fetch our use flag name.
             */
            const UseFlagName & flag() const
            {
                return _flag;
            }

            /**
             * Fetch whether we are a ! flag.
             */
            bool inverse() const
            {
                return _inverse;
            }

            virtual const UseNestAtom * as_use_nest_atom() const PALUDIS_ATTRIBUTE((pure));
    };

    /**
     * A TextNestAtom represents a text entry (for example, a URL for
     * SRC_URI).
     *
     * \ingroup DepResolver
     */
    class TextNestAtom :
        public NestAtom,
        public Visitable<TextNestAtom, NestAtomVisitorTypes>
    {
        private:
            std::string _text;

        public:
            TextNestAtom(const std::string & text);

            /**
             * Destructor.
             */
            ~TextNestAtom();

            /**
             * Fetch the package name.
             */
            const std::string & text() const
            {
                return _text;
            }

            typedef CountedPtr<TextNestAtom, count_policy::InternalCountTag> Pointer;
            typedef CountedPtr<const TextNestAtom, count_policy::InternalCountTag> ConstPointer;
    };

}

#endif
