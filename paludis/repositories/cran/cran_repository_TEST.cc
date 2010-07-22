/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Danny van Dyk
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/dep_spec.hh>
#include <paludis/dep_spec_flattener.hh>
#include <paludis/repositories/cran/cran_dep_parser.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

namespace
{
    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }
}

namespace test_cases
{
    struct CRANRepositoryPackagesTest : TestCase
    {
        CRANRepositoryPackagesTest() : TestCase("PACKAGES") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "cran");
            keys->insert("library", "cran_repository_TEST_dir/library");
            keys->insert("location", "cran_repository_TEST_dir/repo1");
            keys->insert("builddir", "cran_repository_TEST_dir/tmp");
            std::shared_ptr<Repository> repo(CRANRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cran")));
            TEST_CHECK(repo->has_package_named(QualifiedPackageName("cran/testpackage1")));
            TEST_CHECK(repo->has_package_named(QualifiedPackageName("cran/testpackage2")));
        }
    } test_cran_repository_packages;

    struct CRANRepositoryBundleTest: TestCase
    {
        CRANRepositoryBundleTest() : TestCase("Bundle") { }

        void run()
        {
            TestEnvironment env;
            std::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format",   "cran");
            keys->insert("library", "cran_repository_TEST_dir/library");
            keys->insert("location", "cran_repository_TEST_dir/repo2");
            keys->insert("builddir", "cran_repository_TEST_dir/tmp");
            std::shared_ptr<Repository> repo(CRANRepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            TEST_CHECK(repo->has_package_named(QualifiedPackageName("cran/testbundle")));
            TEST_CHECK(repo->has_package_named(QualifiedPackageName("cran/bundlepkg1")));
            TEST_CHECK(repo->has_package_named(QualifiedPackageName("cran/bundlepkg2")));
        }
    } test_cran_repository_bundle;
}

