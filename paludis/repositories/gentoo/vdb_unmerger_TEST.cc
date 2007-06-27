/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

#include "vdb_unmerger.hh"
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    class VDBUnmergerNoDisplay :
        public VDBUnmerger
    {
        protected:

            void display(const std::string &) const
            {
            }

        public:

            VDBUnmergerNoDisplay(const VDBUnmergerOptions & o) :
                VDBUnmerger(o)
            {
            }
    };

    class VDBUnmergerTest :
        public TestCase
    {
        public:

            FSEntry root_dir;
            std::string target;
            TestEnvironment env;
            VDBUnmergerNoDisplay unmerger;

            bool repeatable() const
            {
                return false;
            }

        protected:

            VDBUnmergerTest(const std::string & what) :
                TestCase("unmerge '" + what + "' test"),
                root_dir("vdb_unmerger_TEST_dir/root"),
                target(what),
                unmerger(VDBUnmergerOptions::create()
                        .environment(&env)
                        .root(root_dir)
                        .contents_file("vdb_unmerger_TEST_dir/CONTENTS/" + what)
                        .config_protect("")
                        .config_protect_mask("")
                        .package_id(tr1::shared_ptr<PackageID>()))
            {
            }
    };
}

namespace test_cases
{
    struct VDBUnmergerTestFileOk : VDBUnmergerTest
    {
        VDBUnmergerTestFileOk() : VDBUnmergerTest("file_ok") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_ok;

    struct VDBUnmergerTestFileWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestFileWithSpaces() : VDBUnmergerTest("file_ with spaces") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_file_with_spaces;

    struct VDBUnmergerTestFileBadType : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadType() : VDBUnmergerTest("file_bad_type") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_directory());
        }
    } test_vdb_unmerger_file_bad_type;

    struct VDBUnmergerTestFileBadMd5sum : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadMd5sum() : VDBUnmergerTest("file_bad_md5sum") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_file_bad_md5sum;

    struct VDBUnmergerTestFileBadMtime : VDBUnmergerTest
    {
        VDBUnmergerTestFileBadMtime() : VDBUnmergerTest("file_bad_mtime") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_file_bad_mtime;

    struct VDBUnmergerTestDirOk : VDBUnmergerTest
    {
        VDBUnmergerTestDirOk() : VDBUnmergerTest("dir_ok") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_dir_ok;

    struct VDBUnmergerTestDirWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestDirWithSpaces() : VDBUnmergerTest("dir_ with spaces") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_dir_with_spaces;

    struct VDBUnmergerTestDirBadType : VDBUnmergerTest
    {
        VDBUnmergerTestDirBadType() : VDBUnmergerTest("dir_bad_type") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_dir_bad_type;

    struct VDBUnmergerTestDirNotEmpty : VDBUnmergerTest
    {
        VDBUnmergerTestDirNotEmpty() : VDBUnmergerTest("dir_not_empty") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_directory());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_directory());
        }
    } test_vdb_unmerger_dir_not_empty;

    struct VDBUnmergerTestSymOk : VDBUnmergerTest
    {
        VDBUnmergerTestSymOk() : VDBUnmergerTest("sym_ok") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_ok;

    struct VDBUnmergerTestSymWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestSymWithSpaces() : VDBUnmergerTest("sym_ with spaces") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_sym_with_spaces;

    struct VDBUnmergerTestSymBadType : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadType() : VDBUnmergerTest("sym_bad_type") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_sym_bad_type;

    struct VDBUnmergerTestSymBadDst : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadDst() : VDBUnmergerTest("sym_bad_dst") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());
            TEST_CHECK(! (root_dir / "sym_dst_bad").exists());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_dst;

    struct VDBUnmergerTestSymBadMtime : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadMtime() : VDBUnmergerTest("sym_bad_mtime") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());

        }
    } test_vdb_unmerger_sym_bad_mtime;

    struct VDBUnmergerTestSymBadEntry1 : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadEntry1() : VDBUnmergerTest("sym_bad_entry_1") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_entry_1;

    struct VDBUnmergerTestSymBadEntry2 : VDBUnmergerTest
    {
        VDBUnmergerTestSymBadEntry2() : VDBUnmergerTest("sym_bad_entry_2") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_symbolic_link());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_symbolic_link());
        }
    } test_vdb_unmerger_sym_bad_entry_2;

    struct VDBUnmergerTestFifoOk : VDBUnmergerTest
    {
        VDBUnmergerTestFifoOk() : VDBUnmergerTest("fifo_ok") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_fifo());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_fifo_ok;

    struct VDBUnmergerTestFifoWithSpaces : VDBUnmergerTest
    {
        VDBUnmergerTestFifoWithSpaces() : VDBUnmergerTest("fifo_ with spaces") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_fifo());

            unmerger.unmerge();

            TEST_CHECK(! (root_dir / target).exists());
        }
    } test_vdb_unmerger_fifo_with_spaces;

    struct VDBUnmergerTestFifoBadType : VDBUnmergerTest
    {
        VDBUnmergerTestFifoBadType() : VDBUnmergerTest("fifo_bad_type") { }

        void run()
        {
            TEST_CHECK((root_dir / target).is_regular_file());

            unmerger.unmerge();

            TEST_CHECK((root_dir / target).is_regular_file());
        }
    } test_vdb_unmerger_fifo_bad_type;
}

