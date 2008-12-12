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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_TYPE_LIST_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_TYPE_LIST_HH 1

#include <paludis/util/type_list-fwd.hh>

namespace paludis
{
    struct TypeListTail
    {
    };

    template <typename Item_, typename Tail_>
    struct TypeListEntry
    {
        typedef Item_ Item;
        typedef Tail_ Tail;
    };

    template <typename Item_, typename Tail_>
    struct MakeTypeListEntry
    {
        typedef TypeListEntry<Item_, Tail_> Type;
    };

    template <typename Tail_>
    struct MakeTypeListEntry<TypeListTail, Tail_>
    {
        typedef TypeListTail Type;
    };

    template <
        typename T01_,
        typename T02_,
        typename T03_,
        typename T04_,
        typename T05_,
        typename T06_,
        typename T07_,
        typename T08_,
        typename T09_,
        typename T10_,
        typename T11_,
        typename T12_,
        typename T13_,
        typename T14_,
        typename T15_,
        typename T16_,
        typename T17_,
        typename T18_,
        typename T19_,
        typename T20_,
        typename T21_,
        typename T22_,
        typename T23_,
        typename T24_,
        typename T25_
        >
    struct MakeTypeList
    {
        typedef
            typename MakeTypeListEntry<T01_,
                typename MakeTypeListEntry<T02_,
                    typename MakeTypeListEntry<T03_,
                        typename MakeTypeListEntry<T04_,
                            typename MakeTypeListEntry<T05_,
                                typename MakeTypeListEntry<T06_,
                                    typename MakeTypeListEntry<T07_,
                                        typename MakeTypeListEntry<T08_,
                                            typename MakeTypeListEntry<T09_,
                                                typename MakeTypeListEntry<T10_,
                                                    typename MakeTypeListEntry<T11_,
                                                        typename MakeTypeListEntry<T12_,
                                                            typename MakeTypeListEntry<T13_,
                                                                typename MakeTypeListEntry<T14_,
                                                                    typename MakeTypeListEntry<T15_,
                                                                        typename MakeTypeListEntry<T16_,
                                                                            typename MakeTypeListEntry<T17_,
                                                                                typename MakeTypeListEntry<T18_,
                                                                                    typename MakeTypeListEntry<T19_,
                                                                                        typename MakeTypeListEntry<T20_,
                                                                                            typename MakeTypeListEntry<T21_,
                                                                                                typename MakeTypeListEntry<T22_,
                                                                                                    typename MakeTypeListEntry<T23_,
                                                                                                        typename MakeTypeListEntry<T24_,
                                                                                                            typename MakeTypeListEntry<T25_, TypeListTail>::Type
                                                                                                        >::Type
                                                                                                    >::Type
                                                                                                >::Type
                                                                                            >::Type
                                                                                        >::Type
                                                                                    >::Type
                                                                                >::Type
                                                                            >::Type
                                                                        >::Type
                                                                    >::Type
                                                                >::Type
                                                            >::Type
                                                        >::Type
                                                    >::Type
                                                >::Type
                                            >::Type
                                        >::Type
                                    >::Type
                                >::Type
                            >::Type
                        >::Type
                    >::Type
                >::Type
            >::Type Type;
    };

    template <>
    struct MakeTypeListConstEntry<TypeListTail>
    {
        typedef TypeListTail Type;
    };

    template <typename Item_, typename Tail_>
    struct MakeTypeListConstEntry<TypeListEntry<Item_, Tail_> >
    {
        typedef TypeListEntry<const Item_, typename MakeTypeListConstEntry<Tail_>::Type> Type;
    };

    template <typename TypeList_>
    struct MakeTypeListConst
    {
        typedef typename MakeTypeListConstEntry<TypeList_>::Type Type;
    };
}

#endif
