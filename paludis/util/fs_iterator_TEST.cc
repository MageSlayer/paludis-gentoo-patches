/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Mark Loeser
 * Copyright (c) 2010 Ciaran McCreesh
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

#include <paludis/util/fs_iterator.hh>
#include <paludis/util/fs_path.hh>
#include <paludis/util/fs_error.hh>
#include <paludis/util/options.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct FSIteratorManipulationTest : TestCase
    {
        FSIteratorManipulationTest() : TestCase("construction and manipulation") { }

        void run()
        {
            TEST_CHECK_THROWS(FSIterator(FSPath("/i/dont/exist/"), { }), FSError);

            FSIterator iter(FSPath("fs_iterator_TEST_dir"), { });
            FSIterator iter1(iter);
            TEST_CHECK(iter == iter1);
            TEST_CHECK(!(iter != iter1));
        }
    } test_fs_iterator_manipulation;

    struct FSIteratorIterateTest : TestCase
    {
        FSIteratorIterateTest() : TestCase("iterate") {}

        void run()
        {
            FSIterator iter(FSPath("fs_iterator_TEST_dir"), { });
            FSIterator iter1(FSPath("fs_iterator_TEST_dir"), { });
            FSIterator iter2(FSPath("fs_iterator_TEST_dir"), { fsio_include_dotfiles });
            FSIterator iter3(FSPath("fs_iterator_TEST_dir"), { fsio_inode_sort });

            TEST_CHECK(iter != FSIterator());
            TEST_CHECK(FSIterator() != iter);

            TEST_CHECK_EQUAL(iter->basename(), "file1");
            TEST_CHECK(++iter != FSIterator());
            TEST_CHECK_EQUAL(iter->basename(), "file2");
            TEST_CHECK(++iter != FSIterator());
            TEST_CHECK_EQUAL(iter->basename(), "file4");
            TEST_CHECK(++iter == FSIterator());
            TEST_CHECK(FSIterator() == iter);

            while (iter1 != FSIterator())
                ++iter1;
            TEST_CHECK(iter1 == FSIterator());
            TEST_CHECK(iter == iter1);

            TEST_CHECK_EQUAL(iter2->basename(), ".file3");
            TEST_CHECK(++iter2 != FSIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file1");
            TEST_CHECK(++iter2 != FSIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file2");
            TEST_CHECK(++iter2 != FSIterator());
            TEST_CHECK_EQUAL(iter2->basename(), "file4");
            TEST_CHECK(++iter2 == FSIterator());
            TEST_CHECK(FSIterator() == iter2);
            TEST_CHECK(iter2 == FSIterator());

            TEST_CHECK(iter1 == iter2);
            TEST_CHECK(iter2 == iter1);

            TEST_CHECK_EQUAL(std::distance(iter3, FSIterator()), 3);
        }
    } test_fs_iterator_iterate;
}

