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

#include "deps_visible_check.hh"
#include "check_result.hh"
#include "qa_environment.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/join.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace paludis::qa;
using namespace test;

namespace test_cases
{
    struct DepsVisibleTest
    {
        QAEnvironment env;

        FSEntry basedir() const
        {
            return FSEntry::cwd() / "deps_visible_check_TEST_dir";
        }

        DepsVisibleTest(const std::string & subdir) :
            env(basedir() / subdir)
        {
        }
    };

    struct DepsVisibleNoDepsTest :
        TestCase,
        DepsVisibleTest
    {
        DepsVisibleNoDepsTest() : TestCase("no deps"), DepsVisibleTest("repo1") { }

        void run()
        {
            PerProfileEbuildCheckData params(PerProfileEbuildCheckData::create()
                    .name(QualifiedPackageName("cat-one/pkg-one"))
                    .version(VersionSpec("1"))
                    .environment(&env)
                    .profile(basedir() / "repo1/profiles/profile"));

            CheckResult r((*(*PerProfileEbuildCheckMaker::get_instance()->find_maker(
                            DepsVisibleCheck::identifier()))())(params));

            TestMessageSuffix suffix("r=" + r.item() + ": " + join(r.begin(), r.end(), "; "), false);
            TEST_CHECK(r.empty());
        }
    } deps_visible_no_deps_test;

    struct DepsVisibleOkTest :
        TestCase,
        DepsVisibleTest
    {
        DepsVisibleOkTest() : TestCase("ok"), DepsVisibleTest("repo1") { }

        void run()
        {
            PerProfileEbuildCheckData params(PerProfileEbuildCheckData::create()
                    .name(QualifiedPackageName("cat-one/pkg-one"))
                    .version(VersionSpec("2"))
                    .environment(&env)
                    .profile(basedir() / "repo1/profiles/profile"));

            CheckResult r((*(*PerProfileEbuildCheckMaker::get_instance()->find_maker(
                            DepsVisibleCheck::identifier()))())(params));

            TestMessageSuffix suffix("r=" + r.item() + ": " + join(r.begin(), r.end(), "; "), false);
            TEST_CHECK(r.empty());
        }
    } deps_visible_ok_test;

    struct DepsVisibleNotOkTest :
        TestCase,
        DepsVisibleTest
    {
        DepsVisibleNotOkTest() : TestCase("not ok"), DepsVisibleTest("repo1") { }

        void run()
        {
            PerProfileEbuildCheckData params(PerProfileEbuildCheckData::create()
                    .name(QualifiedPackageName("cat-one/pkg-one"))
                    .version(VersionSpec("3"))
                    .environment(&env)
                    .profile(basedir() / "repo1/profiles/profile"));

            CheckResult r((*(*PerProfileEbuildCheckMaker::get_instance()->find_maker(
                            DepsVisibleCheck::identifier()))())(params));

            TEST_CHECK(! r.empty());
        }
    } deps_visible_not_ok_test;
}

