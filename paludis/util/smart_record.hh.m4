#if 0
ifdef(`__gnu__',`',`errprint(`This is not GNU m4...
')m4exit(1)') include(`misc/generated-file.txt')
dnl vim: set ft=cpp et sw=4 sts=4 :

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

define(`max_record_size', `12')
define(`forloop', `pushdef(`$1', `$2')_forloop(`$1', `$2', `$3', `$4')popdef(`$1')')
define(`_forloop', `$4`'ifelse($1, `$3', , `define(`$1', incr($1))_forloop(`$1', `$2', `$3', `$4')')')
#endif

#ifndef PALUDIS_GUARD_PALUDIS_SMART_RECORD_HH
#define PALUDIS_GUARD_PALUDIS_SMART_RECORD_HH 1

#include <paludis/util/comparison_policy.hh>
#include <paludis/util/compare.hh>
#include <paludis/util/exception.hh>
#include <string>

/** \file
 * SmartRecord declarations.
 *
 * \ingroup grprecords
 */

namespace paludis
{
    namespace comparison_method
    {
        /**
         * Comparisons are done by considering each member in order.
         *
         * \ingroup grprecords
         */
        struct SmartRecordCompareByAllTag
        {
        };

        /**
         * Comparisons are done by considering a specific key.
         *
         * \ingroup grprecords
         */
        template <unsigned key_>
        struct SmartRecordCompareByKeyTag
        {
        };

        /**
         * Comparisons are done by considering a range of keys.
         *
         * \ingroup grprecords
         */
        template <unsigned first_key_, unsigned last_key_>
        struct SmartRecordCompareByKeyRangeTag
        {
        };
    }

    /**
     * Provides the basic typedefs required for a MakeSmartRecord instantiation.
     *
     * \ingroup grprecords
     */
    template <
        typename ComparisonModeTag_ = comparison_mode::NoComparisonTag,
        typename ComparisonMethod_ = comparison_method::SmartRecordCompareByAllTag>
    struct SmartRecordTag
    {
        /**
         * Mode used for ComparisonPolicy.
         *
         * \ingroup grprecords
         */
        typedef ComparisonModeTag_ ComparisonModeTag;

        /**
         * Method used for ComparisonPolicy, should be either
         * comparison_method::SmartRecordCompareByAllTag or
         * comparison_method::SmartRecordCompareByKeyTag .
         *
         * \ingroup grprecords
         */
        typedef ComparisonMethod_ ComparisonMethodTag;
    };

    /**
     * Provides the key information typedefs for a MakeSmartRecord
     * instantiation.
     *
     * \ingroup grprecords
     */
    template <typename E_, unsigned Count_>
    struct SmartRecordKeys
    {
        /**
         * The type of our keys.
         */
        typedef E_ Keys;

        /**
         * The number of keys.
         */
        static const unsigned key_count = Count_;
    };

    template <unsigned Idx_, typename T_>
    struct SmartRecordKey;

forloop(`idx', `0', max_record_size, `
    template <typename T_>
    struct SmartRecordKey<`'idx`', T_>
    {
        typedef T_ KeyType`'idx`';
    };
')

    /**
     * Internal use by SmartRecord.
     *
     * \ingroup grprecords
     */
    namespace smart_record_internals
    {
        template <typename T_>
        struct CharStarToString
        {
            typedef T_ Type;
        };

        template <unsigned i_>
        struct CharStarToString<char [i_]>
        {
            typedef std::string Type;
        };

        template <unsigned i_>
        struct CharStarToString<const char [i_]>
        {
            typedef std::string Type;
        };

        template <>
        struct CharStarToString<char *>
        {
            typedef std::string Type;
        };

        struct ParamListTail
        {
            static const unsigned list_length = 0;
        };

        template <unsigned left_idx_, typename L_, typename R_>
        struct ParamList
        {
            static const unsigned left_idx = left_idx_;
            L_ left;
            typedef L_ LeftType;

            R_ right;
            typedef R_ RightType;

            static const unsigned list_length = 1 + RightType::list_length;

            ParamList(const L_ & l, const R_ & r) :
                left(l),
                right(r)
            {
            }
        };

        template <unsigned idx_, typename Result_, unsigned left_idx_, typename L_, typename R_>
        Result_
        find_list_entry(const ParamList<left_idx_, L_, R_> & list);

        template <bool is_left, unsigned idx_, typename Result_, unsigned left_idx_, typename L_, typename R_>
        struct FindListEntry;

        template <unsigned idx_, typename Result_, unsigned left_idx_, typename L_, typename R_>
        struct FindListEntry<true, idx_, Result_, left_idx_, L_, R_>
        {
            Result_ operator() (const ParamList<left_idx_, L_, R_> & list) const
            {
                return list.left;
            }
        };

        template <unsigned idx_, typename Result_, unsigned left_idx_, typename L_, typename R_>
        struct FindListEntry<false, idx_, Result_, left_idx_, L_, R_>
        {
            Result_ operator() (const ParamList<left_idx_, L_, R_> & list) const
            {
                return find_list_entry<idx_, Result_>(list.right);
            }
        };

        template <unsigned idx_, typename Result_, unsigned left_idx_, typename L_, typename R_>
        Result_
        find_list_entry(const ParamList<left_idx_, L_, R_> & list)
        {
            return FindListEntry<left_idx_ == idx_, idx_, Result_, left_idx_, L_, R_>()(list);
        }

        template <unsigned idx_, typename T_>
        struct ParamListNode
        {
            T_ value;
            static const unsigned idx = idx_;

            ParamListNode(const T_ & t) :
                value(t)
            {
            }
        };

        template <unsigned left_idx_, typename T_, unsigned right_idx_, typename U_>
        ParamList<left_idx_, T_, ParamList<right_idx_, U_, ParamListTail> >
        operator, (const ParamListNode<left_idx_, T_> & t, const ParamListNode<right_idx_, U_> & u)
        {
            return ParamList<left_idx_, T_, ParamList<right_idx_, U_, ParamListTail> >(t.value,
                    ParamList<right_idx_, U_, ParamListTail>(u.value, ParamListTail()));
        }

        template <unsigned left_idx_, typename T_, typename U_, unsigned right_idx_, typename V_>
        ParamList<left_idx_, V_, ParamList<right_idx_, T_, U_> >
        operator, (const ParamList<right_idx_, T_, U_> & t, const ParamListNode<left_idx_, V_> & u)
        {
            return ParamList<left_idx_, V_, ParamList<right_idx_, T_, U_> >(u.value, t);
        }

        template <typename Tag_, unsigned Key_>
        struct GetRecordKeyType;

forloop(`idx', `0', max_record_size, `
        template <typename Tag_>
        struct GetRecordKeyType<Tag_, `'idx`'>
        {
            typedef typename Tag_::KeyType`'idx`' Type;
        };
')

        template <typename Tag_, unsigned key_count_>
        struct RecordBase;

        template <typename Tag_, unsigned key_count_, unsigned Key_>
        struct RecordKeyGetter;

        template <typename Tag_, unsigned key_count_, unsigned Key_>
        struct RecordKeyPointerGetter;

forloop(`idx', `0', max_record_size, `
        template <typename Tag_, unsigned key_count_>
        struct RecordKeyGetter<Tag_, key_count_, `'idx`'>
        {
            static const typename GetRecordKeyType<Tag_, `'idx`'>::Type &
            do_get(const RecordBase<Tag_, key_count_> & r)
            {
                return r._v`'idx`';
            }

            static typename GetRecordKeyType<Tag_, `'idx`'>::Type &
            do_get(RecordBase<Tag_, key_count_> & r)
            {
                return r._v`'idx`';
            }

            static void
            do_set(RecordBase<Tag_, key_count_> & r,
                const typename GetRecordKeyType<Tag_, `'idx`'>::Type & v)
            {
                r._v`'idx`' = v;
            }
        };
')

        template <typename Tag_, unsigned key_count_, typename ComparisonModeTag_, typename ComparisonMethodTag_>
        struct RecordComparisonBase;

        /**
         * RecordComparisonBase: specialisation for comparison_mode::NoComparisonTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, typename ComparisonMethodTag_>
        struct RecordComparisonBase<Tag_, key_count_, comparison_mode::NoComparisonTag, ComparisonMethodTag_> :
            ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::NoComparisonTag, ComparisonMethodTag_>
        {
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::EqualityComparisonTag and
         * comparison_method::SmartRecordCompareByKeyTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned key_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> > : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
            comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >
        {
            public:
                /// Constructor.
                RecordComparisonBase();

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other);
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::FullComparisonTag and
         * comparison_method::SmartRecordCompareByKeyTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned key_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> > : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >
        {
            public:
                /// Constructor.
                RecordComparisonBase();

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other);
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::EqualityComparisonTag and
         * comparison_method::SmartRecordCompareByAllTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByAllTag> : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
            comparison_method::CompareByMemberComparisonFunctionTag>
        {
            protected:
                /// Do the comparison.
                bool compare(const RecordBase<Tag_, key_count_> & other) const;

            public:
                /// Constructor.
                RecordComparisonBase() :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(&RecordComparisonBase::compare)
                {
                }

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other) :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(other)
                {
                }
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::FullComparisonTag and
         * comparison_method::SmartRecordCompareByAllTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByAllTag> : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberComparisonFunctionTag>
        {
            protected:
                /// Do the comparison.
                int compare(const RecordBase<Tag_, key_count_> & other) const;

            public:
                /// Constructor.
                RecordComparisonBase() :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(&RecordComparisonBase::compare)
                {
                }

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other) :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(other)
                {
                }
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::EqualityComparisonTag and
         * comparison_method::SmartRecordCompareByKeyRangeTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned first_key_, unsigned last_key_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByKeyRangeTag<first_key_, last_key_> > : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
            comparison_method::CompareByMemberComparisonFunctionTag>
        {
            protected:
                /// Do the comparison.
                bool compare(const RecordBase<Tag_, key_count_> & other) const;

            public:
                /// Constructor.
                RecordComparisonBase() :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(&RecordComparisonBase::compare)
                {
                }

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other) :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(other)
                {
                }
        };

        /**
         * RecordComparisonBase: specialisation for
         * comparison_mode::FullComparisonTag and
         * comparison_method::SmartRecordCompareByKeyRangeTag.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned first_key_, unsigned last_key_>
        class RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyRangeTag<first_key_, last_key_> > : public
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
            comparison_method::CompareByMemberComparisonFunctionTag>
        {
            protected:
                /// Do the comparison.
                int compare(const RecordBase<Tag_, key_count_> & other) const;

            public:
                /// Constructor.
                RecordComparisonBase() :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(&RecordComparisonBase::compare)
                {
                }

                /// Copy constructor.
                RecordComparisonBase(const RecordComparisonBase & other) :
                    ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                    comparison_method::CompareByMemberComparisonFunctionTag>(other)
                {
                }
        };

forloop(`idx', `1', max_record_size, `
        template<typename Tag_>
        class RecordBase<Tag_, `'idx`'> : public RecordComparisonBase<Tag_, `'idx`',
              typename Tag_::ComparisonModeTag,
              typename Tag_::ComparisonMethodTag>
        {
forloop(`idy', `0', decr(`'idx`'), `
            friend class RecordKeyGetter<Tag_, `'idx`', `'idy`'>;
            friend class RecordKeyPointerGetter<Tag_, `'idx`', `'idy`'>;
')

            private:
forloop(`idy', `0', decr(`'idx`'), `
                typename Tag_::KeyType`'idy`' _v`'idy`';
')

            public:
                ~RecordBase();

                RecordBase(
ifelse(idx, `1', `', `forloop(`idy', `0', decr(decr(idx)), `
                        const typename Tag_::KeyType`'idy`' & p`'idy`',
') ')
                        const typename Tag_::KeyType`'decr(idx)`' & p`'decr(idx)`'
                        ) :
ifelse(idx, `1', `', `forloop(`idy', `0', decr(decr(idx)), `
                    _v`'idy`'(p`'idy`'),
') ')
                    _v`'decr(idx)`'(p`'decr(idx)`')
                {
                }

                RecordBase(const RecordBase<Tag_, `'idx`'> & other) :
                    RecordComparisonBase<Tag_, `'idx`', typename Tag_::ComparisonModeTag,
                        typename Tag_::ComparisonMethodTag>(other),
ifelse(idx, `1', `', `forloop(`idy', `0', decr(decr(idx)), `
                    _v`'idy`'(other._v`'idy`'),
') ')
                    _v`'decr(idx)`'(other._v`'decr(idx)`')
                {
                }

                const RecordBase & operator= (const RecordBase<Tag_, `'idx`'> & other)
                {
forloop(`idy', `0', decr(idx), `
                    _v`'idy`' = other._v`'idy`';
')
                    return *this;
                }

                template <typename Tag_::Keys k_>
                const typename GetRecordKeyType<Tag_, k_>::Type & get() const
                {
                    return RecordKeyGetter<Tag_, `'idx`', k_>::do_get(*this);
                }

                template <typename Tag_::Keys k_>
                typename GetRecordKeyType<Tag_, k_>::Type & get()
                {
                    return RecordKeyGetter<Tag_, `'idx`', k_>::do_get(*this);
                }

                template <typename Tag_::Keys k_>
                void set(const typename GetRecordKeyType<Tag_, k_>::Type & v)
                {
                    return RecordKeyGetter<Tag_, `'idx`', k_>::do_set(*this, v);
                }

                template <typename List_>
                static RecordBase
                create(const List_ & list)
                {
                    return RecordBase(
ifelse(idx, `1', `', `forloop(`idy', `0', decr(decr(idx)), `
                            find_list_entry<`'idy`', typename Tag_::KeyType`'idy`'>(list),
') ')
                            find_list_entry<`'decr(idx)`', typename Tag_::KeyType`'decr(idx)`'>(list)
                            );
                }
        };

        template<typename Tag_>
        RecordBase<Tag_, `'idx`'>::~RecordBase()
        {
        }
')

forloop(`idx', `0', max_record_size, `
        template <typename Tag_, unsigned key_count_>
        struct RecordKeyPointerGetter<Tag_, key_count_, `'idx`'>
        {
            static typename GetRecordKeyType<Tag_, `'idx>::Type
            RecordBase<Tag_, key_count_>::* do_get_pointer()
            {
                return &RecordBase<Tag_, key_count_>::_v`'idx`';
            }
        };
')

        template <typename Tag_, unsigned key_count_, unsigned key_>
        RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> >::RecordComparisonBase() :
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >(
                        RecordKeyPointerGetter<Tag_, key_count_, key_>::do_get_pointer())
        {
        }

        template <typename Tag_, unsigned key_count_, unsigned key_>
        RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> >::RecordComparisonBase(
                    const RecordComparisonBase & other) :
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::FullComparisonTag,
                comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >(other)
        {
        }

        template <typename Tag_, unsigned key_count_, unsigned key_>
        RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> >::RecordComparisonBase() :
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >(
                        RecordKeyPointerGetter<Tag_, key_count_, key_>::do_get_pointer())
        {
        }

        template <typename Tag_, unsigned key_count_, unsigned key_>
        RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByKeyTag<key_> >::RecordComparisonBase(
                    const RecordComparisonBase & other) :
                ComparisonPolicy<RecordBase<Tag_, key_count_>, comparison_mode::EqualityComparisonTag,
                comparison_method::CompareByMemberTag<typename GetRecordKeyType<Tag_, key_>::Type> >(other)
        {
        }

        template <typename Tag_, unsigned key_count_>
        struct DoFullCompareByAll;

forloop(`idx', `1', max_record_size, `
        template <typename Tag_>
        struct DoFullCompareByAll<Tag_, `'idx`'>
        {
            static int do_compare(const RecordBase<Tag_, `'idx`'> * const a,
                    const RecordBase<Tag_, `'idx`'> * const b)
            {
forloop(`idy', `0', decr(`'idx`'), `
                switch (compare(
                        RecordKeyGetter<Tag_, `'idx`', `'idy`'>::do_get(*a),
                        RecordKeyGetter<Tag_, `'idx`', `'idy`'>::do_get(*b)))
                {
                    case -1:
                        return -1;
                    case 1:
                        return 1;
                    case 0:
                        break;
                    default:
                        throw InternalError(PALUDIS_HERE, "Bad value from compare");
                }
')
                return 0;
            }
        };
')

        template <typename Tag_, unsigned key_count_>
        struct DoEqualCompareByAll;

forloop(`idx', `1', max_record_size, `
        template <typename Tag_>
        struct DoEqualCompareByAll<Tag_, `'idx`'>
        {
            static bool do_compare(const RecordBase<Tag_, `'idx`'> * const a,
                    const RecordBase<Tag_, `'idx`'> * const b)
            {
forloop(`idy', `0', decr(`'idx`'), `
                if (RecordKeyGetter<Tag_, `'idx`', `'idy`'>::do_get(*a) !=
                        RecordKeyGetter<Tag_, `'idx`', `'idy`'>::do_get(*b))
                    return false;
')
                return true;
            }
        };
')

        template <typename Tag_, unsigned key_count_>
        int RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByAllTag>::compare(
                    const RecordBase<Tag_, key_count_> & other) const
        {
            return DoFullCompareByAll<Tag_, key_count_>::do_compare(
                    static_cast<const RecordBase<Tag_, key_count_> *>(this), &other);
        }

        template <typename Tag_, unsigned key_count_>
        bool RecordComparisonBase<Tag_, key_count_, comparison_mode::EqualityComparisonTag,
            comparison_method::SmartRecordCompareByAllTag>::compare(
                    const RecordBase<Tag_, key_count_> & other) const
        {
            return DoEqualCompareByAll<Tag_, key_count_>::do_compare(
                    static_cast<const RecordBase<Tag_, key_count_> *>(this), &other);
        }

        /**
         * Do the comparison for a range of keys.
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned from_key_, unsigned count_>
        struct DoFullCompareByKeyRange
        {
            /**
             * Do the comparison.
             *
             * \ingroup grprecords
             */
            static int do_compare(const RecordBase<Tag_, key_count_> * const a,
                    const RecordBase<Tag_, key_count_> * const b)
            {
                if (RecordKeyGetter<Tag_, key_count_, from_key_>::do_get(*a) <
                        RecordKeyGetter<Tag_, key_count_, from_key_>::do_get(*b))
                    return -1;
                if (RecordKeyGetter<Tag_, key_count_, from_key_>::do_get(*a) >
                        RecordKeyGetter<Tag_, key_count_, from_key_>::do_get(*b))
                    return 1;

                return DoFullCompareByKeyRange<Tag_, key_count_, from_key_ + 1, count_ - 1>::do_compare(a, b);
            }
        };

        /**
         * Do the comparison for a range of keys (specialisation for zero sized
         * ranges).
         *
         * \ingroup grprecords
         */
        template <typename Tag_, unsigned key_count_, unsigned from_key_>
        struct DoFullCompareByKeyRange<Tag_, key_count_, from_key_, 0>
        {
            /**
             * Do the comparison.
             */
            static int do_compare(const RecordBase<Tag_, key_count_> * const,
                    const RecordBase<Tag_, key_count_> * const)
            {
                return 0;
            }
        };

#ifndef DOXYGEN
        template <typename Tag_, unsigned key_count_, unsigned from_key_, unsigned to_key_>
        int RecordComparisonBase<Tag_, key_count_, comparison_mode::FullComparisonTag,
            comparison_method::SmartRecordCompareByKeyRangeTag<from_key_, to_key_> >::compare(
                    const RecordBase<Tag_, key_count_> & other) const
        {
            return DoFullCompareByKeyRange<Tag_, key_count_, from_key_, to_key_ - from_key_>::do_compare(
                    static_cast<const RecordBase<Tag_, key_count_> *>(this), &other);
        }
#endif

    }

    /**
     * Create a SmartRecord with the attributes described by Tag_.
     *
     * \ingroup grprecords
     */
    template <typename Tag_>
    struct MakeSmartRecord
    {
        /**
         * The type of our SmartRecord.
         *
         * \ingroup grprecords
         */
        typedef smart_record_internals::RecordBase<Tag_, Tag_::key_count> Type;
    };

    /**
     * Create a named parameter for creating a SmartRecord instance.
     *
     * \ingroup grprecords
     */
    template <unsigned idx_, typename T_>
    smart_record_internals::ParamListNode<idx_, typename smart_record_internals::CharStarToString<T_>::Type>
    param(const T_ & t)
    {
        return smart_record_internals::ParamListNode<idx_,
               typename smart_record_internals::CharStarToString<T_>::Type>(t);
    }
}

#endif
