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

#include "stringify.hh"
#include "config_file.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <sstream>
#include <vector>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for config_file.hh .
 *
 * \ingroup Test
 * \ingroup ConfigFile
 */

#ifndef DOXYGEN
class TestFile : protected ConfigFile
{
    public:
        TestFile(std::istream * const stream) :
            ConfigFile(stream)
        {
            need_lines();
        }

        mutable std::vector<std::string> lines;

    protected:
        void accept_line(const std::string & s) const
        {
            lines.push_back(s);
        }
};
#endif

namespace test_cases
{
    /**
     * \test Test ConfigFile.
     *
     * \ingroup Test
     */
    struct ConfigFileTest : TestCase
    {
        ConfigFileTest() : TestCase("config file") { }

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
            TestFile f(&s);
            TEST_CHECK_EQUAL(f.lines.size(), 4);
            TEST_CHECK_EQUAL(f.lines.at(0), "one");
            TEST_CHECK_EQUAL(f.lines.at(1), "two");
            TEST_CHECK_EQUAL(f.lines.at(2), "three");
            TEST_CHECK_EQUAL(f.lines.at(3), "four  four");
        }
    } test_config_file;
}

