/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/glsa.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct GLSA123456_78Test : TestCase
    {
        GLSA123456_78Test() : TestCase("glsa 123456-78") { }

        void run()
        {
            std::shared_ptr<GLSA> glsa(GLSA::create_from_xml_file("xml_things_TEST_dir/glsa-123456-78.xml"));
            TEST_CHECK(bool(glsa));

            TEST_CHECK_EQUAL("123456-78", glsa->id());
            TEST_CHECK_EQUAL("Kittens: Too Adorable", glsa->title());

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages(), glsa->end_packages()));
            TEST_CHECK_STRINGIFY_EQUAL("animal-feline/kitten", glsa->begin_packages()->name());
            TEST_CHECK_STRINGIFY_EQUAL("0", std::distance(glsa->begin_packages()->begin_archs(),
                        glsa->begin_packages()->end_archs()));

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages()->begin_unaffected(),
                        glsa->begin_packages()->end_unaffected()));
            TEST_CHECK_STRINGIFY_EQUAL("ge", glsa->begin_packages()->begin_unaffected()->op());
            TEST_CHECK_STRINGIFY_EQUAL("1.23", glsa->begin_packages()->begin_unaffected()->version());

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages(), glsa->end_packages()));
            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages()->begin_vulnerable(),
                        glsa->begin_packages()->end_vulnerable()));
            TEST_CHECK_STRINGIFY_EQUAL("lt", glsa->begin_packages()->begin_vulnerable()->op());
            TEST_CHECK_STRINGIFY_EQUAL("1.22", glsa->begin_packages()->begin_vulnerable()->version());
        }
    } glsa_test_123456_78;

    struct GLSA987654_32Test : TestCase
    {
        GLSA987654_32Test() : TestCase("glsa 987654-32") { }

        void run()
        {
            std::shared_ptr<GLSA> glsa(GLSA::create_from_xml_file("xml_things_TEST_dir/glsa-987654-32.xml"));
            TEST_CHECK(bool(glsa));

            TEST_CHECK_EQUAL("987654-32", glsa->id());
            TEST_CHECK_EQUAL("Python: Retarded", glsa->title());

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages(), glsa->end_packages()));
            TEST_CHECK_STRINGIFY_EQUAL("dev-lang/python", glsa->begin_packages()->name());
            TEST_CHECK_STRINGIFY_EQUAL("3", std::distance(glsa->begin_packages()->begin_archs(),
                        glsa->begin_packages()->end_archs()));
            TEST_CHECK_STRINGIFY_EQUAL("mips,sparc,x86", join(glsa->begin_packages()->begin_archs(),
                        glsa->begin_packages()->end_archs(), ","));

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages()->begin_unaffected(),
                        glsa->begin_packages()->end_unaffected()));
            TEST_CHECK_STRINGIFY_EQUAL("ge", glsa->begin_packages()->begin_unaffected()->op());
            TEST_CHECK_STRINGIFY_EQUAL("12.34", glsa->begin_packages()->begin_unaffected()->version());

            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages(), glsa->end_packages()));
            TEST_CHECK_STRINGIFY_EQUAL("1", std::distance(glsa->begin_packages()->begin_vulnerable(),
                        glsa->begin_packages()->end_vulnerable()));
            TEST_CHECK_STRINGIFY_EQUAL("lt", glsa->begin_packages()->begin_vulnerable()->op());
            TEST_CHECK_STRINGIFY_EQUAL("12.34", glsa->begin_packages()->begin_vulnerable()->version());
        }
    } glsa_test_987654_32;
}

