/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#include "stringify.hh"
#include "config_file.hh"
#include "fs_entry.hh"
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

        TestFile(const std::string & filename) :
            ConfigFile(filename)
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

    /**
     * \test Test ConfigFile with file opening.
     *
     * \ingroup Test
     */
    struct ConfigFileOpenFileTest : TestCase
    {
        ConfigFileOpenFileTest() : TestCase("config file open file") { }

        void run()
        {
            FSEntry ff("config_file_TEST_dir/config_file");
            TEST_CHECK(ff.is_regular_file());
            TestFile f(ff);
            TEST_CHECK_EQUAL(f.lines.size(), 1);
            TEST_CHECK_EQUAL(f.lines.at(0), "I am a fish.");

            FSEntry ff2("config_file_TEST_dir/not_a_config_file");
            TEST_CHECK(! ff2.exists());
            TestFile * f2(0);
            TEST_CHECK_THROWS(f2 = new TestFile(ff2), ConfigFileError);

            FSEntry ff3("config_file_TEST_dir/unreadable_file");
            TEST_CHECK(ff3.is_regular_file());
            TestFile * f3(0);
            TEST_CHECK_THROWS(f3 = new TestFile(ff3), ConfigFileError);
        }
    } test_config_file_open_file;
}

