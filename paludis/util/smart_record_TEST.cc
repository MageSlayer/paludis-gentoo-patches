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
#include <vector>
#include <iterator>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for smart_record.hh .
 *
 * \ingroup Test
 */

namespace
{
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

    enum PairKeys
    {
        first,
        second
    };

    struct PairTag :
        SmartRecordTag<comparison_mode::FullComparisonTag, comparison_method::SmartRecordCompareByAllTag>,
        SmartRecordKeys<PairKeys, 2>,
        SmartRecordKey<first, int>,
        SmartRecordKey<second, int>
    {
    };

    typedef MakeSmartRecord<PairTag>::Type Pair;
}

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
            p3.get<age>() = 4;
            TEST_CHECK_EQUAL(p3.get<age>(), 4);
        }
    } test_simple_record;

    /**
     * \test Test a list constructed SmartRecord.
     *
     * \ingroup Test
     */
    struct ListConstructedRecordTest : TestCase
    {
        ListConstructedRecordTest() : TestCase("list constructed") { }

        void run()
        {
            const Person p1(Person::create((
                            param<firstname>("first"),
                            param<surname>("sur"),
                            param<age>(10))));

            TEST_CHECK_EQUAL(p1.get<firstname>(), "first");
            TEST_CHECK_EQUAL(p1.get<surname>(), "sur");
            TEST_CHECK_EQUAL(p1.get<age>(), 10);

            const Person p2(Person::create((
                            param<surname>("bar"),
                            param<firstname>("foo"),
                            param<age>(42))));

            TEST_CHECK_EQUAL(p2.get<firstname>(), "foo");
            TEST_CHECK_EQUAL(p2.get<surname>(), "bar");
            TEST_CHECK_EQUAL(p2.get<age>(), 42);
        }
    } test_list_constructed_record;

    struct PairTest : TestCase
    {
        PairTest() : TestCase("pair") { }

        void run()
        {
            std::vector<Pair> v;
            v.push_back(Pair(0, 0));
            v.push_back(Pair(0, 1));
            v.push_back(Pair(0, 2));
            v.push_back(Pair(1, 0));
            v.push_back(Pair(1, 1));
            v.push_back(Pair(1, 2));
            v.push_back(Pair(2, 0));

            std::vector<Pair>::iterator v1(v.begin()), v_end(v.end());
            for ( ; v1 != v_end ; ++v1)
            {
                TestMessageSuffix s1("v1:" + stringify(v1->get<first>()) + "/"
                        + stringify(v1->get<second>()), true);
                std::vector<Pair>::iterator v2(v.begin());
                for ( ; v2 != v_end ; ++v2)
                {
                    TestMessageSuffix s2("v2:" + stringify(v2->get<first>()) + "/"
                            + stringify(v2->get<second>()), true);

                    if (std::distance(v.begin(), v1) < std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v1 < *v2);
                        TEST_CHECK(*v2 > *v1);
                        TEST_CHECK(*v1 != *v2);
                        TEST_CHECK(*v2 != *v1);
                    }
                    else if (std::distance(v.begin(), v1) > std::distance(v.begin(), v2))
                    {
                        TEST_CHECK(*v2 < *v1);
                        TEST_CHECK(*v1 > *v2);
                        TEST_CHECK(*v2 != *v1);
                        TEST_CHECK(*v1 != *v2);
                    }
                    else
                    {
                        TEST_CHECK(*v2 == *v1);
                        TEST_CHECK(*v1 == *v2);
                    }
                }
            }
        }
    } test_pair;
}

