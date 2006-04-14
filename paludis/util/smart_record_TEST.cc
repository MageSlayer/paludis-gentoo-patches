/* vim: set sw=4 sts=4 et foldmethod=syntax : */

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

#include <paludis/util/smart_record.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for smart_record.hh .
 *
 * \ingroup Test
 */

#ifndef DOXYGEN

enum PersonKeys
{
    firstname,
    surname,
    age
};

struct PersonRecordTag :
    SmartRecordTag<
        comparison_mode::FullComparisonTag,
        comparison_method::SmartRecordCompareByAllTag
    >,
    SmartRecordKeys<PersonKeys, 3>,
    SmartRecordKey<firstname, std::string>,
    SmartRecordKey<surname, std::string>,
    SmartRecordKey<age, unsigned>
{
};

typedef MakeSmartRecord<PersonRecordTag>::Type Person;

#endif

namespace test_cases
{
    /**
     * \test Test a simple SmartRecord.
     *
     * \ingroup Test
     */
    struct SimpleRecordTest : TestCase
    {
        SimpleRecordTest() : TestCase("simple") { }

        void run()
        {
            const Person p1("blah", "first", 10);
            TEST_CHECK_EQUAL(p1.get<firstname>(), "blah");
            TEST_CHECK_EQUAL(p1.get<surname>(), "first");
            TEST_CHECK_EQUAL(p1.get<age>(), 10);

            const Person p2("blah", "second", 6);
            TEST_CHECK_EQUAL(p2.get<firstname>(), "blah");
            TEST_CHECK_EQUAL(p2.get<surname>(), "second");
            TEST_CHECK_EQUAL(p2.get<age>(), 6);

            TEST_CHECK( (p1 <  p2));
            TEST_CHECK( (p1 <= p2));
            TEST_CHECK(!(p1 == p2));
            TEST_CHECK( (p1 != p2));
            TEST_CHECK(!(p1 >= p2));
            TEST_CHECK(!(p1 >  p2));

            Person p3("foo", "bar", 1);
            TEST_CHECK_EQUAL(p3.get<age>(), 1);
            p3.set<age>(2);
            TEST_CHECK_EQUAL(p3.get<age>(), 2);
        }
    } test_simple_record;
}

