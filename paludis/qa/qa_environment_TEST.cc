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

#include "qa_environment.hh"
#include "check_result.hh"
#include "file_check.hh"
#include "create_metadata_check.hh"
#include <paludis/util/fs_entry.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct QAEnvironmentPaludisCommandTest : TestCase
    {
        QAEnvironmentPaludisCommandTest() : TestCase("paludis_command") { }

        void run()
        {
            QAEnvironment env(FSEntry("qa_environment_TEST_dir/repo1"));

            TestMessageSuffix suffix("paludis_command=" + env.paludis_command(), false);

            CheckResult r1((*(*EbuildCheckMaker::get_instance()->find_maker(
                            CreateMetadataCheck::identifier()))())(EbuildCheckData(
                        QualifiedPackageName("cat-one/pkg-one"), VersionSpec("1"), &env)));
            {
                TestMessageSuffix suffix("r1=" + join(r1.begin(), r1.end(), "; "), false);
                TEST_CHECK(r1.empty());
            }

            CheckResult r2((*(*EbuildCheckMaker::get_instance()->find_maker(
                            CreateMetadataCheck::identifier()))())(EbuildCheckData(
                        QualifiedPackageName("cat-one/pkg-one"), VersionSpec("2"), &env)));
            {
                TestMessageSuffix suffix("r2=" + join(r2.begin(), r2.end(), "; "), false);
                TEST_CHECK(! r2.empty());
            }
        }
    } qa_environment_paludis_command_test;
}

