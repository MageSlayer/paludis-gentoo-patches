/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2010, 2012, 2013 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_SINGLETON_IMPL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_SINGLETON_IMPL_HH 1

#include <paludis/util/singleton.hh>
#include <paludis/util/exception.hh>
#include <paludis/util/save.hh>
#include <paludis/util/stringify.hh>
#include <mutex>

template <typename OurType_>
void
paludis::Singleton<OurType_>::_delete(OurType_ * const p)
{
    delete p;
}

template <typename OurType_>
class paludis::Singleton<OurType_>::DeleteOnDestruction
{
    private:
        OurType_ * * const _ptr;

    public:
        DeleteOnDestruction(OurType_ * * const p) :
            _ptr(p)
        {
        }

        ~DeleteOnDestruction()
        {
            paludis::Singleton<OurType_>::_delete(* _ptr);
            * _ptr = 0;
        }
};

template<typename OurType_>
OurType_ * *
paludis::Singleton<OurType_>::_get_instance_ptr()
{
    static OurType_ * instance(0);
    static DeleteOnDestruction delete_instance(&instance);

    return &instance;
}

template<typename OurType_>
OurType_ *
paludis::Singleton<OurType_>::get_instance()
{
    static std::recursive_mutex m;
    std::unique_lock<std::recursive_mutex> lock(m);

    OurType_ * * i(_get_instance_ptr());

    if (0 == *i)
    {
        static bool recursive(false);
        if (recursive)
            throw paludis::InternalError(PALUDIS_HERE, "Recursive instantiation");
        Save<bool> save_recursive(&recursive, true);

        *i = new OurType_;
    }

    return *i;
}

template<typename OurType_>
void
paludis::Singleton<OurType_>::destroy_instance()
{
    OurType_ * * i(_get_instance_ptr());
    delete *i;
    *i = 0;
}

#endif
