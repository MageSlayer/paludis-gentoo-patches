/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_TR1_FUNCTIONAL_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_TR1_FUNCTIONAL_HH 1

#if defined(PALUDIS_TR1_FUNCTIONAL_IS_STD_TR1)

#include <tr1/functional>

namespace paludis
{
    namespace tr1
    {
        using std::tr1::bind;
        using std::tr1::mem_fn;
        using std::tr1::ref;
        using std::tr1::cref;

        namespace placeholders
        {
            using std::tr1::placeholders::_1;
            using std::tr1::placeholders::_2;
            using std::tr1::placeholders::_3;
            using std::tr1::placeholders::_4;
            using std::tr1::placeholders::_5;
            using std::tr1::placeholders::_6;
            using std::tr1::placeholders::_7;
            using std::tr1::placeholders::_8;
            using std::tr1::placeholders::_9;
        }
    }
}

#elif defined(PALUDIS_TR1_FUNCTIONAL_IS_BOOST)

#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/ref.hpp>

namespace paludis
{
    namespace tr1
    {
        using boost::bind;
        using boost::mem_fn;
        using boost::ref;
        using boost::cref;

        namespace placeholders
        {
        }
    }
}

#else
#  error Either PALUDIS_TR1_FUNCTIONAL_IS_STD_TR1 or PALUDIS_TR1_FUNCTIONAL_IS_BOOST should be defined
#endif

#endif
