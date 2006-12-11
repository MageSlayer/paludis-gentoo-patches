/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/gems/cache.hh>
#include <paludis/repositories/gems/gems_repository_exceptions.hh>
#include <paludis/repositories/gems/yaml.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/environment/test/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct GemsRepositoryCacheEntriesTest : TestCase
    {
        GemsRepositoryCacheEntriesTest() : TestCase("cache entries") { }

        void run()
        {
            GemsCache cache(FSEntry("cache_TEST_dir/entries"));

            TEST_CHECK_EQUAL(std::distance(cache.begin(), cache.end()), 2);

            GemsCache::Iterator c(cache.begin());
            TEST_CHECK(c != cache.end());

            TEST_CHECK_EQUAL(c->name, PackageNamePart("foo"));

            ++c;
            TEST_CHECK(c != cache.end());

            TEST_CHECK_EQUAL(c->name, PackageNamePart("bar"));

            ++c;
            TEST_CHECK(c == cache.end());
        }
    } test_cache_entries;

    struct GemsRepositoryCacheNoFileTest : TestCase
    {
        GemsRepositoryCacheNoFileTest() : TestCase("cache no file") { }

        void run()
        {
            TEST_CHECK_THROWS(GemsCache(FSEntry("cache_TEST_dir/nofile")), GemsCacheError);
        }
    } test_cache_no_file;

    struct GemsRepositoryCacheBrokenTest : TestCase
    {
        GemsRepositoryCacheBrokenTest() : TestCase("cache broken file") { }

        void run()
        {
            TEST_CHECK_THROWS(GemsCache(FSEntry("cache_TEST_dir/broken")), YamlError);
        }
    } test_cache_broken;
}



