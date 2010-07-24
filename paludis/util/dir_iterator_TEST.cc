/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser
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

#include <paludis/util/dir_iterator.hh>
#include <paludis/util/options.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for dir_iterator.hh.
 *
 * \ingroup grptestcases
 */

namespace test_cases
{
    /**
     * \test Test DirIterator construction and manipulation.
     *
     * \ingroup grptestcases
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
     *
     * \ingroup grptestcases
     */
    struct DirIteratorIterateTest : TestCase
    {
        DirIteratorIterateTest() : TestCase("iterate") {}

        void run()
        {
            DirIterator iter(FSEntry("dir_iterator_TEST_dir"));
            DirIterator iter1(FSEntry("dir_iterator_TEST_dir"));
            DirIterator iter2(FSEntry("dir_iterator_TEST_dir"), { dio_include_dotfiles });
            DirIterator iter3(FSEntry("dir_iterator_TEST_dir"), { dio_inode_sort });

            TEST_CHECK(iter != DirIterator());
            TEST_CHECK(DirIterator() != iter);

            TEST_CHECK_EQUAL(iter->basename(), "file1");
            TEST_CHECK(++iter != DirIterator());
            TEST_CHECK_EQUAL(iter->basename(), "file2");
            TEST_CHECK(++iter != DirIterator());
            TEST_CHECK_EQUAL(iter->basename(), "file4");
            TEST_CHECK(++iter == DirIterator());
            TEST_CHECK(DirIterator() == iter);

            while (iter1 != DirIterator())
                ++iter1;
            TEST_CHECK(iter1 == DirIterator());
            TEST_CHECK(iter == iter1);

            TEST_CHECK_EQUAL(iter2->basename(), ".file3");
            TEST_CHECK(++iter2 != DirIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file1");
            TEST_CHECK(++iter2 != DirIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file2");
            TEST_CHECK(++iter2 != DirIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file4");
            TEST_CHECK(++iter2 == DirIterator());
            TEST_CHECK(DirIterator() == iter2);
            TEST_CHECK(iter2 == DirIterator());

            TEST_CHECK(iter1 == iter2);
            TEST_CHECK(iter2 == iter1);

            TEST_CHECK_EQUAL(std::distance(iter3, DirIterator()), 3);
        }
    } test_dir_iterator_iterate;
}
