/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "hashed_containers.hh"
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <paludis/name.hh>

using namespace paludis;
using namespace test;

namespace test_cases
{
    /**
     * \test Test CRCHash
     */
    struct CRCHashTest : TestCase
    {
        CRCHashTest() : TestCase("crc hash") { }

        void run()
        {
            TEST_CHECK_EQUAL(std::size_t(47503), (CRCHash<std::string>()("moo") & 0xffff));
        }
    } test_crc_hash;

    /**
     * \test Test HashSet.
     *
     */
    struct HashSetTestQPN : TestCase
    {
        HashSetTestQPN() : TestCase("hash set qpn") { }

        void run()
        {
            MakeHashedSet<QualifiedPackageName>::Type s;
            TEST_CHECK(s.empty());
            TEST_CHECK(s.end() == s.find(QualifiedPackageName("foo/bar")));
            TEST_CHECK(s.insert(QualifiedPackageName("foo/bar")).second);
            TEST_CHECK(s.end() != s.find(QualifiedPackageName("foo/bar")));
            TEST_CHECK(! s.empty());
            TEST_CHECK(! s.insert(QualifiedPackageName("foo/bar")).second);
        }
    } test_hash_set_qpn;
}

