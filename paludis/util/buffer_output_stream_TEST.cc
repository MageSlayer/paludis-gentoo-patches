/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009 Ciaran McCreesh
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

#include <paludis/util/buffer_output_stream.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <sstream>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct BufferOutputStreamTest : TestCase
    {
        BufferOutputStreamTest() : TestCase("buffer_output_stream") { }

        void run()
        {
            BufferOutputStream s;
            std::stringstream t;

            for (int n(0), n_end(1000) ; n != n_end ; ++n)
            {
                s << n << std::endl;
                t << n << std::endl;
            }

            std::stringstream ss;
            s.unbuffer(ss);

            TEST_CHECK_EQUAL(ss.str(), t.str());

            s << "foo" << std::endl;
            std::stringstream sss;
            s.unbuffer(sss);
            TEST_CHECK_EQUAL(sss.str(), "foo\n");
        }
    } test_buffer_output_stream;
}

