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

#ifndef PALUDIS_GUARD_PALUDIS_UTIL_KC_HH
#define PALUDIS_GUARD_PALUDIS_UTIL_KC_HH 1

#include <paludis/util/kc-fwd.hh>
#include <paludis/util/tr1_type_traits.hh>

namespace paludis
{
    namespace kc
    {
        template <bool b_, typename T_>
        struct SV
        {
            template <typename V_>
            static const T_ & sv(const T_ & t, const V_ &)
            {
                return t;
            }
        };

        template <typename T_>
        struct SV<false, T_>
        {
            template <typename D_, typename V_>
            static V_ sv(const D_ &, V_ v)
            {
                return v;
            }
        };

        template <typename Key_, typename Type_>
        struct Field
        {
            typedef Type_ ConstructorType;
            typedef void DefaultConstructorValueType;

            typedef Key_ NamedFirstParamType;
            typedef Type_ NamedSecondParamType;
        };

        template <unsigned id_>
        struct NoField
        {
            typedef NoField<id_> ConstructorType;
            typedef NoField<id_> DefaultConstructorValueType;

            typedef NoField<id_> NamedFirstParamType;
            typedef NoField<id_> NamedSecondParamType;
        };

        template <unsigned id_>
        struct Key
        {
        };

        template <bool b_, typename T_>
        struct NamedField;

        template <typename T_>
        struct NamedField<false, T_>
        {
        };

        template <unsigned id_, typename T_>
        struct NamedField<true, Field<Key<id_>, T_> >
        {
            T_ value;

            NamedField(const T_ & v) :
                value(v)
            {
            }
        };

        template <typename T_>
        struct Part;

        template <unsigned id_>
        struct Part<NoField<id_> >
        {
            public:
                void operator[] (const NoField<id_> &);

                Part(const NoField<id_> &)
                {
                }

                Part(const NamedField<false, NoField<id_> > &)
                {
                }
        };

        template <unsigned id_, typename Type_>
        class Part<Field<Key<id_>, Type_> >
        {
            private:
                Type_ _value;

            public:
                Type_ & operator[] (const Key<id_> &)
                {
                    return _value;
                }

                const Type_ & operator[] (const Key<id_> &) const
                {
                    return _value;
                }

                Part(const Type_ & v) :
                    _value(v)
                {
                }

                Part(const NamedField<true, Field<Key<id_>, Type_> > & v) :
                    _value(v.value)
                {
                }
        };

        template <
            typename T1_,
            typename T2_,
            typename T3_,
            typename T4_,
            typename T5_,
            typename T6_,
            typename T7_,
            typename T8_,
            typename T9_,
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
            typename T20_
            > class KeyedClass :
                public Part<T1_>,
                public Part<T2_>,
                public Part<T3_>,
                public Part<T4_>,
                public Part<T5_>,
                public Part<T6_>,
                public Part<T7_>,
                public Part<T8_>,
                public Part<T9_>,
                public Part<T10_>,
                public Part<T11_>,
                public Part<T12_>,
                public Part<T13_>,
                public Part<T14_>,
                public Part<T15_>,
                public Part<T16_>,
                public Part<T17_>,
                public Part<T18_>,
                public Part<T19_>,
                public Part<T20_>
        {
            public:
                typedef KeyedClass BaseType;

                KeyedClass
                (
                        const typename T1_::ConstructorType & v1 = typename T1_::DefaultConstructorValueType(),
                        const typename T2_::ConstructorType & v2 = typename T2_::DefaultConstructorValueType(),
                        const typename T3_::ConstructorType & v3 = typename T3_::DefaultConstructorValueType(),
                        const typename T4_::ConstructorType & v4 = typename T4_::DefaultConstructorValueType(),
                        const typename T5_::ConstructorType & v5 = typename T5_::DefaultConstructorValueType(),
                        const typename T6_::ConstructorType & v6 = typename T6_::DefaultConstructorValueType(),
                        const typename T7_::ConstructorType & v7 = typename T7_::DefaultConstructorValueType(),
                        const typename T8_::ConstructorType & v8 = typename T8_::DefaultConstructorValueType(),
                        const typename T9_::ConstructorType & v9 = typename T9_::DefaultConstructorValueType(),
                        const typename T10_::ConstructorType & v10 = typename T10_::DefaultConstructorValueType(),
                        const typename T11_::ConstructorType & v11 = typename T11_::DefaultConstructorValueType(),
                        const typename T12_::ConstructorType & v12 = typename T12_::DefaultConstructorValueType(),
                        const typename T13_::ConstructorType & v13 = typename T13_::DefaultConstructorValueType(),
                        const typename T14_::ConstructorType & v14 = typename T14_::DefaultConstructorValueType(),
                        const typename T15_::ConstructorType & v15 = typename T15_::DefaultConstructorValueType(),
                        const typename T16_::ConstructorType & v16 = typename T16_::DefaultConstructorValueType(),
                        const typename T17_::ConstructorType & v17 = typename T17_::DefaultConstructorValueType(),
                        const typename T18_::ConstructorType & v18 = typename T18_::DefaultConstructorValueType(),
                        const typename T19_::ConstructorType & v19 = typename T19_::DefaultConstructorValueType(),
                        const typename T20_::ConstructorType & v20 = typename T20_::DefaultConstructorValueType()
                  ) :
                    Part<T1_>(v1),
                    Part<T2_>(v2),
                    Part<T3_>(v3),
                    Part<T4_>(v4),
                    Part<T5_>(v5),
                    Part<T6_>(v6),
                    Part<T7_>(v7),
                    Part<T8_>(v8),
                    Part<T9_>(v9),
                    Part<T10_>(v10),
                    Part<T11_>(v11),
                    Part<T12_>(v12),
                    Part<T13_>(v13),
                    Part<T14_>(v14),
                    Part<T15_>(v15),
                    Part<T16_>(v16),
                    Part<T17_>(v17),
                    Part<T18_>(v18),
                    Part<T19_>(v19),
                    Part<T20_>(v20)
                {
                }

                KeyedClass(const KeyedClass & other) :
                    Part<T1_>(other),
                    Part<T2_>(other),
                    Part<T3_>(other),
                    Part<T4_>(other),
                    Part<T5_>(other),
                    Part<T6_>(other),
                    Part<T7_>(other),
                    Part<T8_>(other),
                    Part<T9_>(other),
                    Part<T10_>(other),
                    Part<T11_>(other),
                    Part<T12_>(other),
                    Part<T13_>(other),
                    Part<T14_>(other),
                    Part<T15_>(other),
                    Part<T16_>(other),
                    Part<T17_>(other),
                    Part<T18_>(other),
                    Part<T19_>(other),
                    Part<T20_>(other)
                {
                }

                using Part<T1_>::operator[];
                using Part<T2_>::operator[];
                using Part<T3_>::operator[];
                using Part<T4_>::operator[];
                using Part<T5_>::operator[];
                using Part<T6_>::operator[];
                using Part<T7_>::operator[];
                using Part<T8_>::operator[];
                using Part<T9_>::operator[];
                using Part<T10_>::operator[];
                using Part<T11_>::operator[];
                using Part<T12_>::operator[];
                using Part<T13_>::operator[];
                using Part<T14_>::operator[];
                using Part<T15_>::operator[];
                using Part<T16_>::operator[];
                using Part<T17_>::operator[];
                using Part<T18_>::operator[];
                using Part<T19_>::operator[];
                using Part<T20_>::operator[];

                template <
                    bool b1_,
                    bool b2_,
                    bool b3_,
                    bool b4_,
                    bool b5_,
                    bool b6_,
                    bool b7_,
                    bool b8_,
                    bool b9_,
                    bool b10_,
                    bool b11_,
                    bool b12_,
                    bool b13_,
                    bool b14_,
                    bool b15_,
                    bool b16_,
                    bool b17_,
                    bool b18_,
                    bool b19_,
                    bool b20_
                    >
                struct Named
                {
                    NamedField<b1_, T1_> v1;
                    NamedField<b2_, T2_> v2;
                    NamedField<b3_, T3_> v3;
                    NamedField<b4_, T4_> v4;
                    NamedField<b5_, T5_> v5;
                    NamedField<b6_, T6_> v6;
                    NamedField<b7_, T7_> v7;
                    NamedField<b8_, T8_> v8;
                    NamedField<b9_, T9_> v9;
                    NamedField<b10_, T10_> v10;
                    NamedField<b11_, T11_> v11;
                    NamedField<b12_, T12_> v12;
                    NamedField<b13_, T13_> v13;
                    NamedField<b14_, T14_> v14;
                    NamedField<b15_, T15_> v15;
                    NamedField<b16_, T16_> v16;
                    NamedField<b17_, T17_> v17;
                    NamedField<b18_, T18_> v18;
                    NamedField<b19_, T19_> v19;
                    NamedField<b20_, T20_> v20;

                    Named()
                    {
                    }

                    Named(
                            NamedField<b1_, T1_> p1,
                            NamedField<b2_, T2_> p2,
                            NamedField<b3_, T3_> p3,
                            NamedField<b4_, T4_> p4,
                            NamedField<b5_, T5_> p5,
                            NamedField<b6_, T6_> p6,
                            NamedField<b7_, T7_> p7,
                            NamedField<b8_, T8_> p8,
                            NamedField<b9_, T9_> p9,
                            NamedField<b10_, T10_> p10,
                            NamedField<b11_, T11_> p11,
                            NamedField<b12_, T12_> p12,
                            NamedField<b13_, T13_> p13,
                            NamedField<b14_, T14_> p14,
                            NamedField<b15_, T15_> p15,
                            NamedField<b16_, T16_> p16,
                            NamedField<b17_, T17_> p17,
                            NamedField<b18_, T18_> p18,
                            NamedField<b19_, T19_> p19,
                            NamedField<b20_, T20_> p20
                         ) :
                        v1(p1),
                        v2(p2),
                        v3(p3),
                        v4(p4),
                        v5(p5),
                        v6(p6),
                        v7(p7),
                        v8(p8),
                        v9(p9),
                        v10(p10),
                        v11(p11),
                        v12(p12),
                        v13(p13),
                        v14(p14),
                        v15(p15),
                        v16(p16),
                        v17(p17),
                        v18(p18),
                        v19(p19),
                        v20(p20)
                    {
                    }

                    template <typename K_, typename V_>
                    Named<
                        tr1::is_same<K_, typename T1_::NamedFirstParamType>::value ? (true && ! b1_) : b1_,
                        tr1::is_same<K_, typename T2_::NamedFirstParamType>::value ? (true && ! b2_) : b2_,
                        tr1::is_same<K_, typename T3_::NamedFirstParamType>::value ? (true && ! b3_) : b3_,
                        tr1::is_same<K_, typename T4_::NamedFirstParamType>::value ? (true && ! b4_) : b4_,
                        tr1::is_same<K_, typename T5_::NamedFirstParamType>::value ? (true && ! b5_) : b5_,
                        tr1::is_same<K_, typename T6_::NamedFirstParamType>::value ? (true && ! b6_) : b6_,
                        tr1::is_same<K_, typename T7_::NamedFirstParamType>::value ? (true && ! b7_) : b7_,
                        tr1::is_same<K_, typename T8_::NamedFirstParamType>::value ? (true && ! b8_) : b8_,
                        tr1::is_same<K_, typename T9_::NamedFirstParamType>::value ? (true && ! b9_) : b9_,
                        tr1::is_same<K_, typename T10_::NamedFirstParamType>::value ? (true && ! b10_) : b10_,
                        tr1::is_same<K_, typename T11_::NamedFirstParamType>::value ? (true && ! b11_) : b11_,
                        tr1::is_same<K_, typename T12_::NamedFirstParamType>::value ? (true && ! b12_) : b12_,
                        tr1::is_same<K_, typename T13_::NamedFirstParamType>::value ? (true && ! b13_) : b13_,
                        tr1::is_same<K_, typename T14_::NamedFirstParamType>::value ? (true && ! b14_) : b14_,
                        tr1::is_same<K_, typename T15_::NamedFirstParamType>::value ? (true && ! b15_) : b15_,
                        tr1::is_same<K_, typename T16_::NamedFirstParamType>::value ? (true && ! b16_) : b16_,
                        tr1::is_same<K_, typename T17_::NamedFirstParamType>::value ? (true && ! b17_) : b17_,
                        tr1::is_same<K_, typename T18_::NamedFirstParamType>::value ? (true && ! b18_) : b18_,
                        tr1::is_same<K_, typename T19_::NamedFirstParamType>::value ? (true && ! b19_) : b19_,
                        tr1::is_same<K_, typename T20_::NamedFirstParamType>::value ? (true && ! b20_) : b20_
                        >
                        operator() (
                                const K_ &,
                                const V_ & v)
                    {
                        return Named<
                            tr1::is_same<K_, typename T1_::NamedFirstParamType>::value ? true : b1_,
                            tr1::is_same<K_, typename T2_::NamedFirstParamType>::value ? true : b2_,
                            tr1::is_same<K_, typename T3_::NamedFirstParamType>::value ? true : b3_,
                            tr1::is_same<K_, typename T4_::NamedFirstParamType>::value ? true : b4_,
                            tr1::is_same<K_, typename T5_::NamedFirstParamType>::value ? true : b5_,
                            tr1::is_same<K_, typename T6_::NamedFirstParamType>::value ? true : b6_,
                            tr1::is_same<K_, typename T7_::NamedFirstParamType>::value ? true : b7_,
                            tr1::is_same<K_, typename T8_::NamedFirstParamType>::value ? true : b8_,
                            tr1::is_same<K_, typename T9_::NamedFirstParamType>::value ? true : b9_,
                            tr1::is_same<K_, typename T10_::NamedFirstParamType>::value ? true : b10_,
                            tr1::is_same<K_, typename T11_::NamedFirstParamType>::value ? true : b11_,
                            tr1::is_same<K_, typename T12_::NamedFirstParamType>::value ? true : b12_,
                            tr1::is_same<K_, typename T13_::NamedFirstParamType>::value ? true : b13_,
                            tr1::is_same<K_, typename T14_::NamedFirstParamType>::value ? true : b14_,
                            tr1::is_same<K_, typename T15_::NamedFirstParamType>::value ? true : b15_,
                            tr1::is_same<K_, typename T16_::NamedFirstParamType>::value ? true : b16_,
                            tr1::is_same<K_, typename T17_::NamedFirstParamType>::value ? true : b17_,
                            tr1::is_same<K_, typename T18_::NamedFirstParamType>::value ? true : b18_,
                            tr1::is_same<K_, typename T19_::NamedFirstParamType>::value ? true : b19_,
                            tr1::is_same<K_, typename T20_::NamedFirstParamType>::value ? true : b20_
                            >(
                                    SV<tr1::is_same<K_, typename T1_::NamedFirstParamType>::value, typename T1_::NamedSecondParamType>::sv(v, v1),
                                    SV<tr1::is_same<K_, typename T2_::NamedFirstParamType>::value, typename T2_::NamedSecondParamType>::sv(v, v2),
                                    SV<tr1::is_same<K_, typename T3_::NamedFirstParamType>::value, typename T3_::NamedSecondParamType>::sv(v, v3),
                                    SV<tr1::is_same<K_, typename T4_::NamedFirstParamType>::value, typename T4_::NamedSecondParamType>::sv(v, v4),
                                    SV<tr1::is_same<K_, typename T5_::NamedFirstParamType>::value, typename T5_::NamedSecondParamType>::sv(v, v5),
                                    SV<tr1::is_same<K_, typename T6_::NamedFirstParamType>::value, typename T6_::NamedSecondParamType>::sv(v, v6),
                                    SV<tr1::is_same<K_, typename T7_::NamedFirstParamType>::value, typename T7_::NamedSecondParamType>::sv(v, v7),
                                    SV<tr1::is_same<K_, typename T8_::NamedFirstParamType>::value, typename T8_::NamedSecondParamType>::sv(v, v8),
                                    SV<tr1::is_same<K_, typename T9_::NamedFirstParamType>::value, typename T9_::NamedSecondParamType>::sv(v, v9),
                                    SV<tr1::is_same<K_, typename T10_::NamedFirstParamType>::value, typename T10_::NamedSecondParamType>::sv(v, v10),
                                    SV<tr1::is_same<K_, typename T11_::NamedFirstParamType>::value, typename T11_::NamedSecondParamType>::sv(v, v11),
                                    SV<tr1::is_same<K_, typename T12_::NamedFirstParamType>::value, typename T12_::NamedSecondParamType>::sv(v, v12),
                                    SV<tr1::is_same<K_, typename T13_::NamedFirstParamType>::value, typename T13_::NamedSecondParamType>::sv(v, v13),
                                    SV<tr1::is_same<K_, typename T14_::NamedFirstParamType>::value, typename T14_::NamedSecondParamType>::sv(v, v14),
                                    SV<tr1::is_same<K_, typename T15_::NamedFirstParamType>::value, typename T15_::NamedSecondParamType>::sv(v, v15),
                                    SV<tr1::is_same<K_, typename T16_::NamedFirstParamType>::value, typename T16_::NamedSecondParamType>::sv(v, v16),
                                    SV<tr1::is_same<K_, typename T17_::NamedFirstParamType>::value, typename T17_::NamedSecondParamType>::sv(v, v17),
                                    SV<tr1::is_same<K_, typename T18_::NamedFirstParamType>::value, typename T18_::NamedSecondParamType>::sv(v, v18),
                                    SV<tr1::is_same<K_, typename T19_::NamedFirstParamType>::value, typename T19_::NamedSecondParamType>::sv(v, v19),
                                    SV<tr1::is_same<K_, typename T20_::NamedFirstParamType>::value, typename T20_::NamedSecondParamType>::sv(v, v20)
                            );
                    }
                };

                static Named<
                    false, false, false, false, false,
                    false, false, false, false, false,
                    false, false, false, false, false,
                    false, false, false, false, false> named_create()
                {
                    return Named<
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false,
                        false, false, false, false, false>();
                }

                template <
                    bool b1_,
                    bool b2_,
                    bool b3_,
                    bool b4_,
                    bool b5_,
                    bool b6_,
                    bool b7_,
                    bool b8_,
                    bool b9_,
                    bool b10_,
                    bool b11_,
                    bool b12_,
                    bool b13_,
                    bool b14_,
                    bool b15_,
                    bool b16_,
                    bool b17_,
                    bool b18_,
                    bool b19_,
                    bool b20_
                    >
                KeyedClass(const Named<
                        b1_, b2_, b3_, b4_, b5_, b6_, b7_, b8_, b9_,
                        b10_, b11_, b12_, b13_, b14_, b15_, b16_, b17_, b18_, b19_,
                        b20_> & named) :
                    Part<T1_>(named.v1),
                    Part<T2_>(named.v2),
                    Part<T3_>(named.v3),
                    Part<T4_>(named.v4),
                    Part<T5_>(named.v5),
                    Part<T6_>(named.v6),
                    Part<T7_>(named.v7),
                    Part<T8_>(named.v8),
                    Part<T9_>(named.v9),
                    Part<T10_>(named.v10),
                    Part<T11_>(named.v11),
                    Part<T12_>(named.v12),
                    Part<T13_>(named.v13),
                    Part<T14_>(named.v14),
                    Part<T15_>(named.v15),
                    Part<T16_>(named.v16),
                    Part<T17_>(named.v17),
                    Part<T18_>(named.v18),
                    Part<T19_>(named.v19),
                    Part<T20_>(named.v20)
                {
                }
        };

    }
}

#endif
