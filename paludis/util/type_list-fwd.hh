/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_TYPE_LIST_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_TYPE_LIST_FWD_HH 1

namespace paludis
{
    struct TypeListTail;

    template <typename Item_, typename Tail_>
    struct TypeListEntry;

    template <typename Item_, typename Tail_>
    struct MakeTypeListEntry;

    template <
        typename T01_ = TypeListTail,
        typename T02_ = TypeListTail,
        typename T03_ = TypeListTail,
        typename T04_ = TypeListTail,
        typename T05_ = TypeListTail,
        typename T06_ = TypeListTail,
        typename T07_ = TypeListTail,
        typename T08_ = TypeListTail,
        typename T09_ = TypeListTail,
        typename T10_ = TypeListTail,
        typename T11_ = TypeListTail,
        typename T12_ = TypeListTail,
        typename T13_ = TypeListTail,
        typename T14_ = TypeListTail,
        typename T15_ = TypeListTail,
        typename T16_ = TypeListTail,
        typename T17_ = TypeListTail,
        typename T18_ = TypeListTail,
        typename T19_ = TypeListTail,
        typename T20_ = TypeListTail,
        typename T21_ = TypeListTail,
        typename T22_ = TypeListTail,
        typename T23_ = TypeListTail,
        typename T24_ = TypeListTail,
        typename T25_ = TypeListTail
        >
    struct MakeTypeList;

    template <typename TypeList_>
    struct MakeTypeListConst;

    template <typename Item_>
    struct MakeTypeListConstEntry;

}

#endif
