/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
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

#include "line_config_file.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <sstream>
#include <vector>
#include <iterator>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for line_config_file.hh .
 *
 * \ingroup Test
 * \ingroup ConfigFile
 */

namespace test_cases
{
    /**
     * \test Test LineConfigFile.
     *
     * \ingroup Test
     */
    struct LineConfigFileTest : TestCase
    {
        LineConfigFileTest() : TestCase("line config file") { }

        void run()
        {
            std::stringstream s;
            s << "one" << std::endl;
            s << "  two    \t  " << std::endl;
            s << "   \t  " << std::endl;
            s << "" << std::endl;
            s << "three" << std::endl;
            s << "# blah" << std::endl;
            s << "  # blah" << std::endl;
            s << "#" << std::endl;
            s << "  #  \t  " << std::endl;
            s << "four  four" << std::endl;
            LineConfigFile ff(&s);
            std::vector<std::string> f(ff.begin(), ff.end());

            TEST_CHECK_EQUAL(f.size(), 4);
            TEST_CHECK_EQUAL(f.at(0), "one");
            TEST_CHECK_EQUAL(f.at(1), "two");
            TEST_CHECK_EQUAL(f.at(2), "three");
            TEST_CHECK_EQUAL(f.at(3), "four  four");
        }
    } test_line_config_file;
}


