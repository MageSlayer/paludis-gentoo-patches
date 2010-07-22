/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/util/active_object_ptr.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <test/test_concepts.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    typedef ActiveObjectPtr<int *> ActiveIntPtr;
    TESTCASE_SEMIREGULAR(ActiveIntPtr, ActiveIntPtr(new int(10)));

    typedef ActiveObjectPtr<std::shared_ptr<int> > ActiveSharedIntPtr;
    TESTCASE_SEMIREGULAR(ActiveSharedIntPtr, ActiveSharedIntPtr(make_shared_ptr(new int(10))));

    struct TestDereference : TestCase
    {
        TestDereference() : TestCase("dereference") { }

        void run()
        {
            ActiveObjectPtr<std::string *> p(new std::string("monkey"));
            TEST_CHECK_EQUAL(p->length(), 6u);

            ActiveObjectPtr<std::shared_ptr<std::string> > q(
                    make_shared_ptr(new std::string("chimp")));
            TEST_CHECK_EQUAL(q->length(), 5u);
        }
    } test_dereference;

    struct TestValue : TestCase
    {
        TestValue() : TestCase("value") { }

        void run()
        {
            ActiveObjectPtr<std::string *> p(new std::string("monkey"));
            TEST_CHECK_EQUAL(p.value()->length(), 6u);

            ActiveObjectPtr<std::shared_ptr<std::string> > q(
                    make_shared_ptr(new std::string("chimp")));
            TEST_CHECK_EQUAL(q.value()->length(), 5u);
        }
    } test_value;
}

