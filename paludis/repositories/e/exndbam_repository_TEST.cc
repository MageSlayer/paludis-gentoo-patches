/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/exndbam_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    /**
     * \test Test VDBRepository repo names
     *
     */
    struct ExndbamRepositoryRepoNameTest : TestCase
    {
        ExndbamRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exndbam");
            keys->insert("location", "exndbam_repository_TEST_dir/repo1");
            tr1::shared_ptr<Repository> repo(ExndbamRepository::make_exndbam_repository(&env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_exndbam_repository_repo_name;
}

