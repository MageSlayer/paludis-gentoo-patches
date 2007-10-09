/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton <levertond@googlemail.com>
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

#ifndef PALUDIS_GUARD_RECONCILIO_UTIL_ITERATOR_HH
#define PALUDIS_GUARD_RECONCILIO_UTIL_ITERATOR_HH

#include <paludis/util/operators.hh>

#include <iterator>

template <typename Value_, typename Iterator_>
class MemberIterator :
    public std::iterator<std::forward_iterator_tag, const Value_>,
    public paludis::equality_operators::HasEqualityOperators
{
    private:
        Iterator_ _it;
        typedef Value_ std::iterator_traits<Iterator_>::value_type::* Member;
        Member _member;

    public:
        MemberIterator(Iterator_ it, Member member) :
            _it(it),
            _member(member)
        {
        }

        bool operator== (const MemberIterator & other) const
        {
            return _it == other._it;
        }

        MemberIterator & operator++ ()
        {
            ++_it;
            return *this;
        }

        MemberIterator operator++ (int)
        {
            return MemberIterator(_it++, _member);
        }

        typename std::iterator_traits<MemberIterator>::reference operator* () const
        {
            return (*_it).*_member;
        }

        typename std::iterator_traits<MemberIterator>::pointer operator-> () const
        {
            return &((*_it).*_member);
        }
};

template <typename Value_, typename Iterator_>
inline MemberIterator<Value_, Iterator_>
member_iterator(const Iterator_ & it, Value_ std::iterator_traits<Iterator_>::value_type::* member)
{
    return MemberIterator<Value_, Iterator_>(it, member);
}

template <typename Iterator_>
inline MemberIterator<typename std::iterator_traits<Iterator_>::value_type::first_type, Iterator_>
first_iterator(Iterator_ it)
{
    return member_iterator(it, &std::iterator_traits<Iterator_>::value_type::first);
}

template <typename Iterator_>
inline MemberIterator<typename std::iterator_traits<Iterator_>::value_type::second_type, Iterator_>
second_iterator(Iterator_ it)
{
    return member_iterator(it, &std::iterator_traits<Iterator_>::value_type::second);
}

#endif

