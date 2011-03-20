/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2009, 2011 Ciaran McCreesh
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

#include <paludis/util/wrapped_forward_iterator-impl.hh>
#include <paludis/util/join.hh>
#include <paludis/util/iterator_funcs.hh>

#include <vector>
#include <list>

#include <gtest/gtest.h>

using namespace paludis;

namespace paludis
{
    template <>
    struct WrappedForwardIteratorTraits<void>
    {
        typedef std::list<int>::iterator UnderlyingIterator;
    };
}

TEST(WrappedForwardIterator, Works)
{
    std::list<int> l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);

    typedef WrappedForwardIterator<void, int> I;
    ASSERT_EQ("1, 2, 3", join(I(l.begin()), I(l.end()), ", "));
    ASSERT_TRUE(I(l.begin()).underlying_iterator<std::list<int>::iterator>() == l.begin());
}

