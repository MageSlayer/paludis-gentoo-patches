/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_ACTIVE_OBJECT_PTR_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_ACTIVE_OBJECT_PTR_HH 1

#include <paludis/util/active_object_ptr-fwd.hh>
#include <paludis/util/mutex.hh>
#include <memory>

namespace paludis
{
    template <typename T_>
    class ActiveObjectPtr
    {
        private:
            T_ _ptr;
            std::shared_ptr<Mutex> _mutex;

            class Deref
            {
                private:
                    const ActiveObjectPtr * _ptr;
                    std::shared_ptr<Lock> _lock;

                public:
                    Deref(const ActiveObjectPtr * p) :
                        _ptr(p),
                        _lock(std::make_shared<Lock>(*p->_mutex))
                    {
                    }

                    const T_ & operator-> () const
                    {
                        return _ptr->_ptr;
                    }
            };

            friend class Deref;

        public:
            ActiveObjectPtr(const T_ & t) :
                _ptr(t),
                _mutex(std::make_shared<Mutex>())
            {
            }

            ActiveObjectPtr(const ActiveObjectPtr & other) :
                _ptr(other._ptr),
                _mutex(other._mutex)
            {
            }

            ~ActiveObjectPtr()
            {
            }

            ActiveObjectPtr &
            operator= (const ActiveObjectPtr & other)
            {
                if (this != &other)
                {
                    _ptr = other._ptr;
                    _mutex = other._mutex;
                }
                return *this;
            }

            Deref operator-> () const
            {
                return Deref(this);
            }

            const T_ & value() const
            {
                return Deref(this).operator->();
            }
    };
}

#endif
