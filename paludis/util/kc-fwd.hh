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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_KC_FWD_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_KC_FWD_HH 1

namespace paludis
{
    namespace kc
    {
        template <typename Key_, typename Type_>
        struct Field;

        template <unsigned id_>
        struct NoField;

        template <unsigned id_>
        struct Key;

        template <bool b_, typename T_>
        struct NamedField;

        template <typename T_>
        struct NamedField<false, T_>;

        template <unsigned id_, typename T_>
        struct NamedField<true, Field<Key<id_>, T_> >;

        template <typename T_>
        struct Part;

        template <unsigned id_>
        struct Part<NoField<id_> >;

        template <unsigned id_, typename Type_>
        struct Part<Field<Key<id_>, Type_> >;

        template <
            typename T1_ = NoField<1>,
            typename T2_ = NoField<2>,
            typename T3_ = NoField<3>,
            typename T4_ = NoField<4>,
            typename T5_ = NoField<5>,
            typename T6_ = NoField<6>,
            typename T7_ = NoField<7>,
            typename T8_ = NoField<8>,
            typename T9_ = NoField<9>,
            typename T10_ = NoField<10>,
            typename T11_ = NoField<11>,
            typename T12_ = NoField<12>,
            typename T13_ = NoField<13>,
            typename T14_ = NoField<14>,
            typename T15_ = NoField<15>,
            typename T16_ = NoField<16>,
            typename T17_ = NoField<17>,
            typename T18_ = NoField<18>,
            typename T19_ = NoField<19>,
            typename T20_ = NoField<20>
                > class KeyedClass;

    }
}

#endif
