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

#include <paludis/util/tail_output_stream.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    struct TailOutputStreamTest : TestCase
    {
        TailOutputStreamTest() : TestCase("tail output stream") { }

        void run()
        {
            TailOutputStream s(5);

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(false));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "");
            }

            s << "one" << std::endl;

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(false));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "one");
            }

            s << "two" << std::endl;
            s << "three" << std::endl;
            s << "four" << std::endl;
            s << "five" << std::endl;

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(false));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "one/two/three/four/five");
            }

            s << "six" << std::endl;

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(true));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "two/three/four/five/six");
            }

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(false));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "");
            }

            s << "seven" << std::endl;
            s << "eight" << std::endl;

            {
                std::shared_ptr<const Sequence<std::string> > a(s.tail(false));
                TEST_CHECK_EQUAL(join(a->begin(), a->end(), "/"), "seven/eight");
            }
        }
    } test_tail_output_stream;
}

