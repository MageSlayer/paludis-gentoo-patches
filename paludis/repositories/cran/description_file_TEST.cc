/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/repositories/cran/description_file.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <sstream>

using namespace paludis;
using namespace paludis::cranrepository;
using namespace test;

namespace test_cases
{
    struct DescriptionTest : TestCase
    {
        DescriptionTest() : TestCase("description test") { }

        void run()
        {
            std::stringstream s;
            s << "Foo: bar bar" << std::endl;
            s << "  black sheep" << std::endl;
            s << "Moo: cow" << std::endl;

            DescriptionFile f(s);
            TEST_CHECK_EQUAL(f.get("Foo"), "bar bar black sheep");
            TEST_CHECK_EQUAL(f.get("Moo"), "cow");
            TEST_CHECK_EQUAL(f.get("Oink"), "");
        }
    } test_description;
}

