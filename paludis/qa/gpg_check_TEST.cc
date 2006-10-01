/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda <ferdy@gentoo.org>
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

#include <paludis/qa/gpg_check.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fd_holder.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct GPGCheckTest : TestCase
    {
        GPGCheckTest() : TestCase("signed Manifest") { }

        bool skip() const
        {
            FDHolder dev_null(::open("/dev/null", O_WRONLY));
            set_run_command_stdout_fds(dev_null, -1);
            set_run_command_stderr_fds(dev_null, -1);

            return (0 != run_command("gpg --help"));
        }

        void run()
        {
            FSEntry e("gpg_check_TEST_dir");
            TEST_CHECK(e.exists());
            TEST_CHECK(e.is_directory());

            FSEntry package(e / "cat" / "not-signed");
            TEST_CHECK(package.exists());
            TEST_CHECK(package.is_directory());

            CheckResult r((*(*PackageDirCheckMaker::get_instance()->find_maker(
                                GPGCheck::identifier()))())(package));
            TEST_CHECK(! r.empty());

        }
    } test_gpg_check;
}
