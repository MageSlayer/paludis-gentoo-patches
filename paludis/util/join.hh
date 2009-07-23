/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006, 2007, 2009 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_JOIN_HH
#define PALUDIS_GUARD_PALUDIS_JOIN_HH 1

#include <paludis/util/stringify.hh>
#include <iterator>
#include <string>

/** \file
 * Declarations for the join function.
 *
 * \ingroup g_strings
 *
 * \section Examples
 *
 * - None at this time.
 */

namespace paludis
{
    /**
     * Join together the items from i to end using joiner.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename T_>
    T_ join(I_ i, I_ end, const T_ & joiner)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += stringify(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    /**
     * Join together the items from i to end using joiner, using
     * a function other than stringify.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename T_, typename F_>
    T_ join(I_ i, I_ end, const T_ & joiner, const F_ & f)
    {
        T_ result;
        if (i != end)
            while (true)
            {
                result += (f)(*i);
                if (++i == end)
                    break;
                result += joiner;
            }
        return result;
    }

    /**
     * Convenience alternative join allowing a char * to be used for a
     * string.
     *
     * \ingroup g_strings
     */
    template <typename I_>
    std::string join(I_ begin, const I_ end, const char * const t)
    {
        return join(begin, end, std::string(t));
    }

    /**
     * Convenience alternative join allowing a char * to be used for a
     * string, using a function other than stringify.
     *
     * \ingroup g_strings
     */
    template <typename I_, typename F_>
    std::string join(I_ begin, const I_ end, const char * const t, const F_ & f)
    {
        return join(begin, end, std::string(t), f);
    }
}

#endif
