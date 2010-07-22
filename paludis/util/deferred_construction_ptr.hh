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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_DEFERRED_CONSTRUCTION_PTR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_DEFERRED_CONSTRUCTION_PTR_HH 1

#include <paludis/util/deferred_construction_ptr-fwd.hh>
#include <functional>

namespace paludis
{
    template <typename T_>
    class DeferredConstructionPtr
    {
        private:
            mutable T_ _ptr;
            std::function<T_ ()> _f;
            mutable bool _done;

        public:
            DeferredConstructionPtr(const std::function<T_ ()> & f) :
                _ptr(),
                _f(f),
                _done(false)
            {
            }

            DeferredConstructionPtr(const DeferredConstructionPtr & other) :
                _ptr(other._ptr),
                _f(other._f),
                _done(other._done)
            {
            }

            DeferredConstructionPtr &
            operator= (const DeferredConstructionPtr & other)
            {
                if (this != &other)
                {
                    _ptr = other._ptr;
                    _f = other._f;
                    _done = other._done;
                }
                return *this;
            }

            T_ operator-> () const
            {
                if (! _done)
                {
                    _ptr = _f();
                    _done = true;
                }

                return _ptr;
            }

            T_ value() const
            {
                return operator-> ();
            }
    };
}

#endif
