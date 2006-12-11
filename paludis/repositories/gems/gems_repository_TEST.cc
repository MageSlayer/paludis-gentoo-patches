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

#include <paludis/repositories/gems/gems_repository.hh>
#include <paludis/repositories/gems/make_gems_repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/environment/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct GemsRepositoryRepoNameTest : TestCase
    {
        GemsRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            AssociativeCollection<std::string, std::string>::Pointer keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "gems");
            keys->insert("location", "gems_repository_TEST_dir/repo1");
            GemsRepository::Pointer repo(make_gems_repository(
                        &env, env.package_database().raw_pointer(), keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "gems");
        }
    } test_portage_repository_repo_name;
}


