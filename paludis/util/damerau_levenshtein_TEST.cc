/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Fernando J. Pereda
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

#include <paludis/util/damerau_levenshtein.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

/** \file
 * Test cases for damerau_levenshtein.hh
 */

namespace test_cases
{
    /**
     * \test Test DamerauLevenshtain methods
     */
    struct DamerauLevenshteinTest : TestCase
    {
        DamerauLevenshteinTest() : TestCase("Damerau-Levenshtein distance") {}

        void run()
        {
            DamerauLevenshtein dl("foo");

            TEST_CHECK_EQUAL(dl.distance_with("foo"),  0u);
            TEST_CHECK_EQUAL(dl.distance_with("foo1"), 1u);
            TEST_CHECK_EQUAL(dl.distance_with("fo"),   1u);
            TEST_CHECK_EQUAL(dl.distance_with("fao"),  1u);
            TEST_CHECK_EQUAL(dl.distance_with("ofo"),  1u);
            TEST_CHECK_EQUAL(dl.distance_with("fie"),  2u);
            TEST_CHECK_EQUAL(dl.distance_with("ife"),  3u);
            TEST_CHECK_EQUAL(dl.distance_with("bar"),  3u);
            TEST_CHECK_EQUAL(dl.distance_with(""),     3u);

            DamerauLevenshtein de("");

            TEST_CHECK_EQUAL(de.distance_with("foo"),  3u);
            TEST_CHECK_EQUAL(de.distance_with(""),     0u);
        }
    } test_damerau_levenshtein_test;
}
