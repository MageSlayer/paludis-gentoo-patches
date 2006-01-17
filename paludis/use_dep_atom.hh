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

#ifndef PALUDIS_GUARD_PALUDIS_USE_DEP_ATOM_HH
#define PALUDIS_GUARD_PALUDIS_USE_DEP_ATOM_HH 1

#include <paludis/composite_dep_atom.hh>
#include <paludis/use_flag_name.hh>

/** \file
 * Declarations for the UseDepAtom class.
 *
 * \ingroup DepResolver
 */

namespace paludis
{
    class DepAtomVisitor;

    /**
     * Represents a use? ( ) dependency atom.
     *
     * \ingroup DepResolver
     */
    class UseDepAtom : public CompositeDepAtom,
                       public Visitable<UseDepAtom, DepAtomVisitor>
    {
        private:
            const UseFlagName _flag;
            const bool _inverse;

        public:
            /**
             * Constructor.
             */
            UseDepAtom(const UseFlagName &, bool);

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
    };
}

#endif
