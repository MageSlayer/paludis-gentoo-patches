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

namespace paludis
{
    namespace kc
    {
        template <typename Key_, typename Type_>
        struct Field
        {
            typedef Type_ ConstructorType;
            typedef void DefaultConstructorValueType;

            typedef Key_ NamedKeyFirstParamType;
            typedef Type_ NamedKeySecondParamType;
        };

        template <unsigned id_>
        struct NoField
        {
            typedef NoField<id_> ConstructorType;
            typedef NoField<id_> DefaultConstructorValueType;

            typedef NoField<id_> NamedKeyFirstParamType;
            typedef NoField<id_> NamedKeySecondParamType;
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
            const T_ & value;

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
            typename T5_
            > class KeyedClass :
                public Part<T1_>,
                public Part<T2_>,
                public Part<T3_>,
                public Part<T4_>,
                public Part<T5_>
        {
            public:
                typedef KeyedClass BaseType;

                KeyedClass
                (
                        const typename T1_::ConstructorType & v1 = typename T1_::DefaultConstructorValueType(),
                        const typename T2_::ConstructorType & v2 = typename T2_::DefaultConstructorValueType(),
                        const typename T3_::ConstructorType & v3 = typename T3_::DefaultConstructorValueType(),
                        const typename T4_::ConstructorType & v4 = typename T4_::DefaultConstructorValueType(),
                        const typename T5_::ConstructorType & v5 = typename T5_::DefaultConstructorValueType()
                  ) :
                    Part<T1_>(v1),
                    Part<T2_>(v2),
                    Part<T3_>(v3),
                    Part<T4_>(v4),
                    Part<T5_>(v5)
                {
                }

                KeyedClass(const KeyedClass & other) :
                    Part<T1_>(other),
                    Part<T2_>(other),
                    Part<T3_>(other),
                    Part<T4_>(other),
                    Part<T5_>(other)
                {
                }

                using Part<T1_>::operator[];
                using Part<T2_>::operator[];
                using Part<T3_>::operator[];
                using Part<T4_>::operator[];
                using Part<T5_>::operator[];

                template <
                    bool b1_,
                    bool b2_,
                    bool b3_,
                    bool b4_,
                    bool b5_
                    >
                struct Named
                {
                    NamedField<b1_, T1_> v1;
                    NamedField<b2_, T2_> v2;
                    NamedField<b3_, T3_> v3;
                    NamedField<b4_, T4_> v4;
                    NamedField<b5_, T5_> v5;

                    Named()
                    {
                    }

                    Named(
                            NamedField<b1_, T1_> p1,
                            NamedField<b2_, T2_> p2,
                            NamedField<b3_, T3_> p3,
                            NamedField<b4_, T4_> p4,
                            NamedField<b5_, T5_> p5
                         ) :
                        v1(p1),
                        v2(p2),
                        v3(p3),
                        v4(p4),
                        v5(p5)
                    {
                    }

                    Named<true, b2_, b3_, b4_, b5_> operator() (
                            const typename T1_::NamedKeyFirstParamType &,
                            const typename T1_::NamedKeySecondParamType & v)
                    {
                        return Named<! b1_, b2_, b3_, b4_, b5_>(v, v2, v3, v4, v5);
                    }

                    Named<b1_, true, b3_, b4_, b5_> operator() (
                            const typename T2_::NamedKeyFirstParamType &,
                            const typename T2_::NamedKeySecondParamType & v)
                    {
                        return Named<b1_, ! b2_, b3_, b4_, b5_>(v1, v, v3, v4, v5);
                    }

                    Named<b1_, b2_, true, b4_, b5_> operator() (
                            const typename T3_::NamedKeyFirstParamType &,
                            const typename T3_::NamedKeySecondParamType & v)
                    {
                        return Named<b1_, b2_, ! b3_, b4_, b5_>(v1, v2, v, v4, v5);
                    }

                    Named<b1_, b2_, b3_, true, b5_> operator() (
                            const typename T4_::NamedKeyFirstParamType &,
                            const typename T4_::NamedKeySecondParamType & v)
                    {
                        return Named<b1_, b2_, b3_, ! b4_, b5_>(v1, v2, v3, v, v5);
                    }

                    Named<b1_, b2_, b3_, b4_, true> operator() (
                            const typename T5_::NamedKeyFirstParamType &,
                            const typename T5_::NamedKeySecondParamType & v)
                    {
                        return Named<b1_, b2_, b3_, b4_, ! b5_>(v1, v2, v3, v4, v);
                    }
                };

                static Named<false, false, false, false, false> named_create()
                {
                    return Named<false, false, false, false, false>();
                }

                template <
                    bool b1_,
                    bool b2_,
                    bool b3_,
                    bool b4_,
                    bool b5_>
                KeyedClass(const Named<b1_, b2_, b3_, b4_, b5_> & named) :
                    Part<T1_>(named.v1),
                    Part<T2_>(named.v2),
                    Part<T3_>(named.v3),
                    Part<T4_>(named.v4),
                    Part<T5_>(named.v5)
                {
                }
        };

    }
}

#endif
