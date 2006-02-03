/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser <halcy0n@gentoo.org>
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

#include "dir_iterator.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for dir_iterator.hh.
 *
 * \ingroup Test
 */

namespace test_cases
{
    /**
     * \test Test DirIterator construction and manipulation.
     */
    struct DirIteratorManipulationTest : TestCase
    {
        DirIteratorManipulationTest() : TestCase("construction and manipulation") { }

        void run()
        {
            TEST_CHECK_THROWS(DirIterator(FSEntry("/i/dont/exist/")), DirOpenError);

            DirIterator iter(FSEntry("dir_iterator_TEST_dir"));
            DirIterator iter1(iter);
            TEST_CHECK(iter == iter1);
            TEST_CHECK(!(iter != iter1));
        }
    } test_dir_iterator_manipulation;

    /**
     * \test Test DirIterator iterating abilities
     */
    struct DirIteratorIterateTest : TestCase
    {
        DirIteratorIterateTest() : TestCase("iterate") {}

        void run()
        {
            DirIterator iter(FSEntry("dir_iterator_TEST_dir"));
            DirIterator iter1(FSEntry("dir_iterator_TEST_dir"));

            TEST_CHECK(iter != DirIterator());
            TEST_CHECK(DirIterator() != iter);

            TEST_CHECK(*iter == FSEntry("dir_iterator_TEST_dir/all_perms"));
            TEST_CHECK(*(iter++) == FSEntry("dir_iterator_TEST_dir/all_perms"));
            TEST_CHECK(iter == DirIterator());
            TEST_CHECK(DirIterator() == iter);

            TEST_CHECK(++iter1 == DirIterator());
        }
    } test_dir_iterator_iterate;
}
