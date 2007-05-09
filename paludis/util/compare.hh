/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_COMPARE_HH
#define PALUDIS_GUARD_PALUDIS_COMPARE_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/validated.hh>
#include <string>

/** \file
 * Declarations for the compare functions.
 *
 * \ingroup grpcompare
 */

namespace paludis
{
    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    inline int compare(int t1, int t2)
    {
        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        else
            return 0;
    }

    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    inline int compare(unsigned t1, unsigned t2)
    {
        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        else
            return 0;
    }

    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    inline int compare(unsigned long t1, unsigned long t2)
    {
        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        else
            return 0;
    }

    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    inline int compare(long t1, long t2)
    {
        if (t1 < t2)
            return -1;
        else if (t1 > t2)
            return 1;
        else
            return 0;
    }

    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    template <typename T_>
    inline int compare(
            const std::basic_string<T_> & t1,
            const std::basic_string<T_> & t2)
    {
        register int r(t1.compare(t2));
        if (r < 0)
            return -1;
        else if (r > 0)
            return 1;
        else
            return 0;
    }

    /**
     * Compare t1 and t2.
     *
     * This is deliberately not specialised for the three parameter Validated.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    template <typename T_, typename U_>
    inline int compare(
            const Validated<T_, U_> & t1,
            const Validated<T_, U_> & t2)
    {
        return compare(t1.data(), t2.data());
    }

    /**
     * Compare t1 and t2.
     *
     * \retval -1 if t1 < t2, 1 if t1 > t2, 0 otherwise.
     *
     * \ingroup grpcompare
     */
    template <typename T_>
    int compare(
            const T_ & t1,
            const T_ & t2)
    {
        if (t1 < t2)
            return -1;
        else if (t2 < t1)
            return 1;
        else
            return 0;
    }
}

#endif
