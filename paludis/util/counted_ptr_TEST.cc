/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2005, 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include <paludis/util/counted_ptr.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <string>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for counted_ptr.hh .
 *
 * \ingroup Test
 * \ingroup Pointer
 */

#ifndef DOXYGEN
class MyClass : public InternalCounted<MyClass>
{
    private:
        int _v;

    public:
        MyClass(const int v) :
            _v(v)
        {
        }

        MyClass(const MyClass & other) :
            InternalCounted<MyClass>(),
            _v(other._v)
        {
        }

        const MyClass & operator= (const MyClass & other)
        {
            _v = other._v;
            return *this;
        }

        bool operator== (const MyClass & other) const
        {
            return _v == other._v;
        }

        int value() const
        {
            return _v;
        }
};

std::ostream & operator<< (std::ostream & s, const MyClass & c)
{
    s << c.value();
    return s;
}
#endif

namespace test_cases
{
    /**
     * \test CountedPtr creation tests.
     *
     * \ingroup Test
     */
    struct CountedPtrCreationTests : TestCase
    {
        CountedPtrCreationTests() : TestCase("CountedPtr creation tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            CountedPtr<std::string, count_policy::ExternalCountTag> j(new std::string("moo"));
        }
    } test_counted_ptr_creation;

    /**
     * \test CountedPtr dereference tests.
     *
     * \ingroup Test
     */
    struct CountedPtrDereferenceTests : TestCase
    {
        CountedPtrDereferenceTests() : TestCase("CountedPtr dereference tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            TEST_CHECK_EQUAL(*i, 10);

            CountedPtr<std::string, count_policy::ExternalCountTag> j(new std::string("moo"));
            TEST_CHECK_EQUAL(*j, "moo");
            TEST_CHECK_EQUAL(j->length(), 3);
        }
    } test_counted_ptr_dereference;

    /**
     * \test CountedPtr copy tests.
     *
     * \ingroup Test
     */
    struct CountedPtrCopyTests : TestCase
    {
        CountedPtrCopyTests() : TestCase("CountedPtr copy tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            TEST_CHECK_EQUAL(*i, 10);

            CountedPtr<int, count_policy::ExternalCountTag> i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);
        }
    } test_counted_ptr_copy;

    /**
     * \test CountedPtr dereference-assign tests.
     *
     * \ingroup Test
     */
    struct CountedPtrDereferenceAssignTests : TestCase
    {
        CountedPtrDereferenceAssignTests() : TestCase("CountedPtr dereference assign tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            TEST_CHECK_EQUAL(*i, 10);
            *i = 20;
            TEST_CHECK_EQUAL(*i, 20);

            CountedPtr<int, count_policy::ExternalCountTag> i2(i);
            TEST_CHECK_EQUAL(*i, 20);
            TEST_CHECK_EQUAL(*i2, 20);

            *i = 30;
            TEST_CHECK_EQUAL(*i, 30);
            TEST_CHECK_EQUAL(*i2, 30);

            *i2 = 40;
            TEST_CHECK_EQUAL(*i, 40);
            TEST_CHECK_EQUAL(*i2, 40);
        }
    } test_counted_ptr_dereference_assign;

    /**
     * \test CountedPtr assign value tests.
     *
     * \ingroup Test
     */
    struct CountedPtrAssignValueTests : TestCase
    {
        CountedPtrAssignValueTests() : TestCase("CountedPtr assign value tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            TEST_CHECK_EQUAL(*i, 10);
            CountedPtr<int, count_policy::ExternalCountTag> i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);

            i = CountedPtr<int, count_policy::ExternalCountTag>(new int(20));
            TEST_CHECK_EQUAL(*i, 20);
            TEST_CHECK_EQUAL(*i2, 10);
        }
    } test_counted_ptr_assign_value;

    /**
     * \test CountedPtr assign pointer tests.
     *
     * \ingroup Test
     */
    struct CountedPtrAssignPointerTests : TestCase
    {
        CountedPtrAssignPointerTests() : TestCase("CountedPtr assign pointer tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag> i(new int(10));
            TEST_CHECK_EQUAL(*i, 10);
            CountedPtr<int, count_policy::ExternalCountTag> i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);

            CountedPtr<int, count_policy::ExternalCountTag> i3(new int(30));

            i = i3;
            TEST_CHECK_EQUAL(*i, 30);
            TEST_CHECK_EQUAL(*i2, 10);
            TEST_CHECK_EQUAL(*i3, 30);

            i.assign(new int(50));
            TEST_CHECK_EQUAL(*i, 50);

            i.zero();
            TEST_CHECK(! i);
        }
    } test_counted_ptr_assign_pointer;

    /**
     * \test CountedPtr internal creation tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalCreationTests : TestCase
    {
        CountedPtrInternalCreationTests() : TestCase("CountedPtr internal creation tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
        }
    } test_counted_ptr_internal_creation;

    /**
     * \test CountedPtr internal dereference tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalDereferenceTests : TestCase
    {
        CountedPtrInternalDereferenceTests() : TestCase("CountedPtr internal dereference tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
            TEST_CHECK_EQUAL(*i, 10);

            MyClass::Pointer j(new MyClass(20));
            TEST_CHECK_EQUAL(*j, 20);
        }
    } test_counted_ptr_internal_dereference;

    /**
     * \test CountedPtr internal copy tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalCopyTests : TestCase
    {
        CountedPtrInternalCopyTests() : TestCase("CountedPtr internal copy tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
            TEST_CHECK_EQUAL(*i, 10);

            MyClass::Pointer i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);
        }
    } test_counted_ptr_internal_copy;

    /**
     * \test CountedPtr internal dereference-assign tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalDereferenceAssignTests : TestCase
    {
        CountedPtrInternalDereferenceAssignTests() :
            TestCase("CountedPtr internal dereference assign tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
            TEST_CHECK_EQUAL(*i, 10);
            *i = 20;
            TEST_CHECK_EQUAL(*i, 20);

            MyClass::Pointer i2(i);
            TEST_CHECK_EQUAL(*i, 20);
            TEST_CHECK_EQUAL(*i2, 20);

            *i = 30;
            TEST_CHECK_EQUAL(*i, 30);
            TEST_CHECK_EQUAL(*i2, 30);

            *i2 = 40;
            TEST_CHECK_EQUAL(*i, 40);
            TEST_CHECK_EQUAL(*i2, 40);
        }
    } test_counted_ptr_internal_dereference_assign;

    /**
     * \test CountedPtr internal assign value tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalAssignValueTests : TestCase
    {
        CountedPtrInternalAssignValueTests() :
            TestCase("CountedPtr internal assign value tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
            TEST_CHECK_EQUAL(*i, 10);
            MyClass::Pointer i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);

            i = MyClass::Pointer(new MyClass(20));
            TEST_CHECK_EQUAL(*i, 20);
            TEST_CHECK_EQUAL(*i2, 10);
        }
    } test_counted_ptr_internal_assign_value;

    /**
     * \test CountedPtr internal assign pointer tests.
     *
     * \ingroup Test
     */
    struct CountedPtrInternalAssignPointerTests : TestCase
    {
        CountedPtrInternalAssignPointerTests() :
            TestCase("CountedPtr internal assign pointer tests") { }

        void run()
        {
            MyClass::Pointer i(new MyClass(10));
            TEST_CHECK_EQUAL(*i, 10);
            MyClass::Pointer i2(i);
            TEST_CHECK_EQUAL(*i, 10);
            TEST_CHECK_EQUAL(*i2, 10);

            MyClass::Pointer i3(new MyClass(30));

            i = i3;
            TEST_CHECK_EQUAL(*i, 30);
            TEST_CHECK_EQUAL(*i2, 10);
            TEST_CHECK_EQUAL(*i3, 30);

            i.assign(new MyClass(50));
            TEST_CHECK_EQUAL(*i, 50);

            i.zero();
            TEST_CHECK(! i);
        }
    } test_counted_ptr_internal_assign_pointer;

    /**
     * \test CountedPtr zero dereferences.
     *
     * \ingroup Test
     */
    struct CountedPtrZeroDeferenceTests : TestCase
    {
        CountedPtrZeroDeferenceTests() :
            TestCase("CountedPtr zero dereference tests") { }

        void run()
        {
            CountedPtr<int, count_policy::ExternalCountTag, dereference_policy::CheckedDereferenceTag> i(0);
            TEST_CHECK_THROWS(++*i, CountedPtrError);
        }
    } test_counted_ptr_zero_dereference;
}

