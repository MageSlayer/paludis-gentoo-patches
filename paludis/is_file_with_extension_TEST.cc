/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "is_file_with_extension.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <vector>
#include <algorithm>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct IsFileWithExtensionTest : TestCase
    {
        IsFileWithExtensionTest() : TestCase("is file with extension") { }

        void run()
        {
            IsFileWithExtension a("foo");
            IsFileWithExtension b("cc");

            FSEntry c("teh.foo");
            FSEntry d("is_file_with_extension_TEST.cc");

            TEST_CHECK( !a(c) );
            TEST_CHECK( !a(d) );
            TEST_CHECK( !b(c) );
            TEST_CHECK( b(d) );

        }
    } test_is_file_with_extension;

    struct IsFileWithExtensionPrefixTest : TestCase
    {
        IsFileWithExtensionPrefixTest() : TestCase("is file with extension (with prefix)") { }

        void run()
        {
            IsFileWithExtension a("teh","foo");
            IsFileWithExtension b("is", "cc");
            IsFileWithExtension c("with", "cc");

            FSEntry d("teh.foo");
            FSEntry e("is_file_with_extension_TEST.cc");

            TEST_CHECK( !a(d) );
            TEST_CHECK( !a(e) );
            TEST_CHECK( !b(d) );
            TEST_CHECK( b(e) );
            TEST_CHECK( !c(e) );

        }
    } test_is_file_with_extension_prefix;


}

