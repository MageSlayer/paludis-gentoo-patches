/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH
#define PALUDIS_GUARD_PALUDIS_PRIVATE_IMPLEMENTATION_PATTERN_HH 1

/** \file
 * Declarations for the Pimp pattern.
 *
 * \ingroup g_oo
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Private implementation data, to be specialised for any class that
     * uses Pimp.
     *
     * \ingroup g_oo
     */
    template <typename C_>
    struct Imp;

    /**
     * Pointer to our implementation data.
     *
     * \ingroup g_oo
     * \since 0.58
     */
    template <typename C_>
    class Pimp
    {
        private:
            Imp<C_> * _ptr;

        public:
            ///\name Basic operations
            ///\{

            template <typename... Args_>
            explicit Pimp(Args_ && ... args);

            ~Pimp();

            Pimp(Pimp &&);

            Pimp(const Pimp &) = delete;
            Pimp & operator= (const Pimp &) = delete;

            ///\}

            ///\name Dereference operators
            //\{

            inline Imp<C_> * operator-> ();

            inline const Imp<C_> * operator-> () const;

            Imp<C_> * get();

            const Imp<C_> * get() const;

            ///\}

            /**
             * Reset to a new Imp.
             */
            void reset(Imp<C_> * p);
    };
}

template <typename C_>
paludis::Imp<C_> *
paludis::Pimp<C_>::operator-> ()
{
    return _ptr;
}

template <typename C_>
const paludis::Imp<C_> *
paludis::Pimp<C_>::operator-> () const
{
    return _ptr;
}

#endif
