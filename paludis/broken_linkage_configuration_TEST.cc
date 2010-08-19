/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 David Leverton
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

#include <paludis/broken_linkage_configuration.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <paludis/util/wrapped_forward_iterator.hh>

#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <unistd.h>
#include <cstdlib>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct ConfigurationTest : TestCase
    {
        ConfigurationTest() : TestCase("configuration") {}

        void run()
        {
            setenv("SEARCH_DIRS", "/quuxlib", 1);
            setenv("SEARCH_DIRS_MASK", "/quuxlib/quux", 1);
            setenv("LD_LIBRARY_MASK", "libquux.so", 1);

            BrokenLinkageConfiguration config(FSEntry::cwd() / "broken_linkage_configuration_TEST_dir");

            TEST_CHECK_EQUAL(join(config.begin_search_dirs(), config.end_search_dirs(), " "),
                             "/42 /alib /barbin /barlib/foo /bazbin /bin /blib /fhqwhgads1 /fhqwhgads2 /foobin /foolib/bar /hwdp foobar /lib32 /lib64 /quuxlib /qwerty1 /qwerty2 /sbin /uiop1 /uiop2 /usr/bin /usr/lib* /usr/sbin");
            TEST_CHECK_EQUAL(join(config.begin_ld_so_conf(), config.end_ld_so_conf(), " "),
                             "/42 /barlib/foo /fhqwhgads1 /fhqwhgads2 /foolib/bar /lib /qwerty1 /qwerty2 /uiop1 /uiop2 /usr/lib hwdp foobar");

            TEST_CHECK(config.dir_is_masked(FSEntry("/meh")));
            TEST_CHECK(config.dir_is_masked(FSEntry("/quuxlib/quux")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/feh")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/opt/OpenOffice")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/usr/lib/openoffice")));
            TEST_CHECK(! config.dir_is_masked(FSEntry("/foo")));

            TEST_CHECK(config.lib_is_masked("libquux.so"));
            TEST_CHECK(config.lib_is_masked("libxyzzy.so"));
            TEST_CHECK(config.lib_is_masked("libodbcinst.so"));
            TEST_CHECK(config.lib_is_masked("libodbc.so"));
            TEST_CHECK(config.lib_is_masked("libjava.so"));
            TEST_CHECK(config.lib_is_masked("libjvm.so"));
            TEST_CHECK(! config.lib_is_masked("libfoo.so"));
        }
    } configuration_test;
}

