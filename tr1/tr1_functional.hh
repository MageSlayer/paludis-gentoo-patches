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

#ifndef PALUDIS_GUARD_TR1_TR1_FUNCTIONAL_HH
#define PALUDIS_GUARD_TR1_TR1_FUNCTIONAL_HH 1

/*
 * Used if we have boost but not std::tr1::bind<> etc.
 */

#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>

namespace std
{
    namespace tr1
    {
        using boost::bind;
        using boost::mem_fn;

        namespace placeholders
        {
            using _1;
            using _2;
            using _3;
            using _4;
            using _5;
            using _6;
            using _7;
            using _8;
            using _9;
        }
    }
}

#endif
