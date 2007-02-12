/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include "merger.hh"
#include <paludis/environment/test/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <fstream>
#include <iterator>

using namespace paludis;
using namespace test;

namespace
{
    struct TestMerger :
        Merger
    {
        TestMerger(const MergerOptions & opts) :
            Merger(opts)
        {
        }

        void record_install_file(const FSEntry &, const FSEntry &, const std::string &)
        {
        }

        void record_install_dir(const FSEntry &, const FSEntry &)
        {
        }

        void record_install_sym(const FSEntry &, const FSEntry &)
        {
        }

        void on_error(bool is_check, const std::string & s)
        {
            if (is_check)
                make_check_fail();
            else
                throw MergerError(s);
        }

        void on_warn(bool, const std::string &)
        {
        }

        bool config_protected(const FSEntry &, const FSEntry &)
        {
            return false;
        }

        std::string make_config_protect_name(const FSEntry & src, const FSEntry &)
        {
            return src.basename() + ".cfgpro";
        }
    };

    class MergerTest :
        public TestCase
    {
        public:
            FSEntry image_dir;
            FSEntry root_dir;
            TestEnvironment env;
            TestMerger merger;

            bool repeatable() const
            {
                return false;
            }

        protected:
            MergerTest(MergerEntryType src_type, MergerEntryType dst_type, int n = 0) :
                TestCase("merge " + stringify(src_type) + " over " + stringify(dst_type) + (0 == n ? "" : " "
                            + stringify(n))),
                image_dir("merger_TEST_dir/image_" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n))),
                root_dir("merger_TEST_dir/root_" + stringify(src_type) + "_over_" + stringify(dst_type)
                        + (0 == n ? "" : "_" + stringify(n))),
                merger(MergerOptions::create()
                        .image(image_dir)
                        .root(root_dir)
                        .environment(&env))
            {
            }
    };
}

namespace test_cases
{
    struct MergerTestSymNothing : MergerTest
    {
        MergerTestSymNothing() : MergerTest(met_sym, met_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "sym").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
        }
    } test_merger_sym_nothing;

    struct MergerTestSymSym : MergerTest
    {
        MergerTestSymSym() : MergerTest(met_sym, met_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "root_dst");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
        }
    } test_merger_sym_sym;

    struct MergerTestSymFile : MergerTest
    {
        MergerTestSymFile() : MergerTest(met_sym, met_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_regular_file());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "sym").is_symbolic_link());
            TEST_CHECK_EQUAL((root_dir / "sym").readlink(), "image_dst");
        }
    } test_merger_sym_file;

    struct MergerTestSymDir : MergerTest
    {
        MergerTestSymDir() : MergerTest(met_sym, met_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "sym").is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "sym").is_directory());
        }
    } test_merger_sym_dir;

    struct MergerTestDirNothing : MergerTest
    {
        MergerTestDirNothing() : MergerTest(met_dir, met_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "dir").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_directory());
        }
    } test_merger_dir_nothing;

    struct MergerTestDirDir : MergerTest
    {
        MergerTestDirDir() : MergerTest(met_dir, met_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_directory());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_directory());
        }
    } test_merger_dir_dir;

    struct MergerTestDirFile : MergerTest
    {
        MergerTestDirFile() : MergerTest(met_dir, met_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_regular_file());
        }
    } test_merger_dir_file;

    struct MergerTestDirSym1 : MergerTest
    {
        MergerTestDirSym1() : MergerTest(met_dir, met_sym, 1) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_directory());
            TEST_CHECK(! (root_dir / "dir" / "file").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_directory());
            TEST_CHECK((root_dir / "dir" / "file").is_regular_file());
        }
    } test_merger_dir_sym_1;

    struct MergerTestDirSym2 : MergerTest
    {
        MergerTestDirSym2() : MergerTest(met_dir, met_sym, 2) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_regular_file());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK((root_dir / "dir").realpath().is_regular_file());
        }
    } test_merger_dir_sym_2;

    struct MergerTestDirSym3 : MergerTest
    {
        MergerTestDirSym3() : MergerTest(met_dir, met_sym, 3) { }

        void run()
        {
            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "dir").is_symbolic_link());
            TEST_CHECK_THROWS((root_dir / "dir").realpath(), FSError);
        }
    } test_merger_dir_sym_3;

    struct MergerTestFileNothing : MergerTest
    {
        MergerTestFileNothing() : MergerTest(met_file, met_nothing) { }

        void run()
        {
            TEST_CHECK(! (root_dir / "file").exists());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").is_regular_file());
            std::ifstream f(stringify(root_dir / "file").c_str());
            TEST_CHECK(f);
            std::string fs(std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>()));
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_nothing;

    struct MergerTestFileFile : MergerTest
    {
        MergerTestFileFile() : MergerTest(met_file, met_file) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").is_regular_file());
            std::ifstream b(stringify(root_dir / "file").c_str());
            TEST_CHECK(b);
            std::string bs((std::istreambuf_iterator<char>(b)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(bs, "root contents\n");

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file").is_regular_file());
            std::ifstream f(stringify(root_dir / "file").c_str());
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image contents\n");
        }
    } test_merger_file_file;

    struct MergerTestFileSym : MergerTest
    {
        MergerTestFileSym() : MergerTest(met_file, met_sym) { }

        void run()
        {
            TEST_CHECK((root_dir / "file1").is_symbolic_link());
            TEST_CHECK((root_dir / "file2").is_symbolic_link());
            TEST_CHECK((root_dir / "file3").is_symbolic_link());

            TEST_CHECK(merger.check());
            merger.merge();

            TEST_CHECK((root_dir / "file1").is_regular_file());
            std::ifstream f(stringify(root_dir / "file1").c_str());
            TEST_CHECK(f);
            std::string fs((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs, "image 1 contents\n");

            TEST_CHECK((root_dir / "file2").is_regular_file());
            std::ifstream f2(stringify(root_dir / "file2").c_str());
            TEST_CHECK(f2);
            std::string fs2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs2, "image 2 contents\n");

            TEST_CHECK((root_dir / "file3").is_regular_file());
            std::ifstream f3(stringify(root_dir / "file3").c_str());
            TEST_CHECK(f3);
            std::string fs3((std::istreambuf_iterator<char>(f3)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(fs3, "image 3 contents\n");
        }
    } test_merger_file_sym;

    struct MergerTestFileDir : MergerTest
    {
        MergerTestFileDir() : MergerTest(met_file, met_dir) { }

        void run()
        {
            TEST_CHECK((root_dir / "file").is_directory());

            TEST_CHECK(! merger.check());
            TEST_CHECK_THROWS(merger.merge(), MergerError);

            TEST_CHECK((root_dir / "file").is_directory());
        }
    } test_merger_file_dir;
}

