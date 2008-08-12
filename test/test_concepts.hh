/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_TEST_TEST_CONCEPTS_HH
#define PALUDIS_GUARD_TEST_TEST_CONCEPTS_HH 1

namespace test
{
}

#define TEST_XSTRINGIFY(x) #x
#define TEST_STRINGIFY(x) TEST_XSTRINGIFY(x)

#define TESTCASE_SEMIREGULAR(Type, initial_instance) \
    struct Type ## IsSemiRegularTest : TestCase \
    { \
        Type ## IsSemiRegularTest() : TestCase(TEST_STRINGIFY(Type) " is semi regular") { } \
        \
        void run() \
        { \
            Type copy(initial_instance); \
            Type assigned = initial_instance; \
            assigned = initial_instance; \
            Type * free_store = new Type(initial_instance); \
            delete free_store; \
        } \
    } Type ## _is_semi_regular_test

#define TESTCASE_STRINGIFYABLE(Type, initial_instance, str) \
    struct Type ## IsStringifiableTest : TestCase \
    { \
        Type ## IsStringifiableTest() : TestCase(TEST_STRINGIFY(Type) " is stringifiable") { } \
        \
        void run() \
        { \
            TEST_CHECK_STRINGIFY_EQUAL(stringify(initial_instance), str); \
        } \
    } Type ## _is_stringifiable_test

#endif
