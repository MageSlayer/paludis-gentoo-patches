/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/host_tuple_name.hh>
#include <paludis/util/exception.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for HostTupleName.
 *
 */

namespace test_cases
{
    /**
     * \test Basic HostTupleName tests.
     *
     */
    struct HostTupleNameTest : TestCase
    {
        HostTupleNameTest() : TestCase("basic") { }

        void run()
        {
            HostTupleName a("i686-unknown-linux-gnu");
            TEST_CHECK(true);
        }
    } test_configuration_name;

    /**
     * \test Validate HostTupleName tests.
     *
     */
    struct HostTupleNameValidateTest : TestCase
    {
        HostTupleNameValidateTest() : TestCase("validate") { }

        void run()
        {
            HostTupleName a("i686-unknown-linux-gnu");
            TEST_CHECK_THROWS(a = HostTupleName("moo!"), HostTupleNameError);
            TEST_CHECK_THROWS(a = HostTupleName("foo-bar!"), HostTupleNameError);
            TEST_CHECK_THROWS(a = HostTupleName("foo-bar-baz-too-many"), HostTupleNameError);
        }
    } test_configuration_name_validate;

    /**
     * \test Compare HostTupleName tests.
     *
     */
    struct HostTupleNameCompareTest : TestCase
    {
        HostTupleNameCompareTest() : TestCase("compare") { }

        void run()
        {
            HostTupleName foo1("i386-pc-linux-gnu");
            HostTupleName foo2("i686-unknown-freebsd6.0");
            HostTupleName foo3("spu-elf");
            HostTupleName foo4("i386-pc-linux-gnu");

            TEST_CHECK( (foo1 <  foo2));
            TEST_CHECK( (foo2 <  foo3));
            TEST_CHECK( (foo1 <  foo3));

            TEST_CHECK( (foo1 == foo4));
        }
    } test_configuration_name_compare;

    /**
     * \test Test ArchitectureNamePart creation.
     *
     */
    struct ArchitectureNamePartCreationTest : public TestCase
    {
        ArchitectureNamePartCreationTest() : TestCase("architecture creation") { }

        void run()
        {
            ArchitectureNamePart a("hppa1.1");
            TEST_CHECK(true);
        }
    } architecture_name_part_creation;

    /**
     * \test Test ArchitectureNamePart validation
     *
     */
    struct ArchitectureNamePartValidationTest : public TestCase
    {
        ArchitectureNamePartValidationTest() : TestCase("architecture validation") {}

        void run()
        {
            ArchitectureNamePart a = ArchitectureNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");

            TEST_CHECK_THROWS(a = ArchitectureNamePart(""), ArchitectureNamePartError);
            TEST_CHECK_THROWS(a = ArchitectureNamePart("*"), ArchitectureNamePartError);
            TEST_CHECK_THROWS(a = ArchitectureNamePart("foo bar"), ArchitectureNamePartError);
        }
    } architecture_name_part_validation;

    /**
     * \test Test ManufacturerNamePart creation.
     *
     */
    struct ManufacturerNamePartCreationTest : public TestCase
    {
        ManufacturerNamePartCreationTest() : TestCase("manufacturer creation") { }

        void run()
        {
            ManufacturerNamePart m1("unknown");
            ManufacturerNamePart m2("");
            TEST_CHECK(true);
        }
    } manufacturer_name_part_creation;

    /**
     * \test Test ManufacturerNamePart validation
     *
     */
    struct ManufacturerNamePartValidationTest : public TestCase
    {
        ManufacturerNamePartValidationTest() : TestCase("manufacturer validation") {}

        void run()
        {
            ManufacturerNamePart m = ManufacturerNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");

            TEST_CHECK_THROWS(m = ManufacturerNamePart("."), ManufacturerNamePartError);
            TEST_CHECK_THROWS(m = ManufacturerNamePart("*"), ManufacturerNamePartError);
            TEST_CHECK_THROWS(m = ManufacturerNamePart("foo bar"), ManufacturerNamePartError);
        }
    } manufacturer_name_part_validation;

    /**
     * \test Test KernelNamePart creation.
     *
     */
    struct KernelNamePartCreationTest : public TestCase
    {
        KernelNamePartCreationTest() : TestCase("kernel creation") { }

        void run()
        {
            KernelNamePart k1("freebsd6.0");
            KernelNamePart k2("");
            TEST_CHECK(true);
        }
    } kernel_name_part_creation;

    /**
     * \test Test KernelNamePart validation
     *
     */
    struct KernelNamePartValidationTest : public TestCase
    {
        KernelNamePartValidationTest() : TestCase("kernel validation") {}

        void run()
        {
            KernelNamePart k = KernelNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");

            TEST_CHECK_THROWS(k = KernelNamePart("aixsucks!"), KernelNamePartError);
            TEST_CHECK_THROWS(k = KernelNamePart("*"), KernelNamePartError);
            TEST_CHECK_THROWS(k = KernelNamePart("foo bar"), KernelNamePartError);
        }
    } kernel_name_part_validation;

    /**
     * \test Test UserlandNamePart creation.
     *
     */
    struct UserlandNamePartCreationTest : public TestCase
    {
        UserlandNamePartCreationTest() : TestCase("userland creation") { }

        void run()
        {
            UserlandNamePart u("freebsd6.0");
            TEST_CHECK(true);
        }
    } userland_name_part_creation;

    /**
     * \test Test UserlandNamePart validation
     *
     */
    struct UserlandNamePartValidationTest : public TestCase
    {
        UserlandNamePartValidationTest() : TestCase("userland validation") {}

        void run()
        {
            UserlandNamePart u = UserlandNamePart("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");

            TEST_CHECK_THROWS(u = UserlandNamePart(""), UserlandNamePartError);
            TEST_CHECK_THROWS(u = UserlandNamePart("*"), UserlandNamePartError);
            TEST_CHECK_THROWS(u = UserlandNamePart("foo bar"), UserlandNamePartError);
        }
    } userland_name_part_validation;

}

