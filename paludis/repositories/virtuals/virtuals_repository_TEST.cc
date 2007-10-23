/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh
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

#include <test/test_runner.hh>
#include <test/test_framework.hh>

#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/virtuals/virtuals_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/query.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace test;
using namespace paludis;

namespace test_cases
{
    struct VirtualsRepositoryTest : TestCase
    {
        VirtualsRepositoryTest() : TestCase("virtuals repository") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<VirtualsRepository> virtuals(new VirtualsRepository(&env));
            tr1::shared_ptr<FakeRepository> repo(new FakeRepository(&env, RepositoryName("repo")));
            tr1::shared_ptr<FakeInstalledRepository> installed(new FakeInstalledRepository(&env, RepositoryName("installed")));

            TEST_CHECK(repo->virtuals_interface);

            env.package_database()->add_repository(2, virtuals);
            env.package_database()->add_repository(3, repo);
            env.package_database()->add_repository(4, installed);

            repo->add_version("cat", "pkg", "1")->provide_key()->set_from_string("virtual/pkg");
            repo->add_version("cat", "pkg", "2")->provide_key()->set_from_string("virtual/pkg");
            repo->add_virtual_package(QualifiedPackageName("virtual/pkg"), make_shared_ptr(new PackageDepSpec(">=cat/pkg-2", pds_pm_permissive)));

            TEST_CHECK(repo->virtual_packages());
            TEST_CHECK_EQUAL(std::distance(repo->virtual_packages()->begin(), repo->virtual_packages()->end()), 1);

            TEST_CHECK(virtuals->has_category_named(CategoryNamePart("virtual")));
            TEST_CHECK(virtuals->has_package_named(QualifiedPackageName("virtual/pkg")));

            tr1::shared_ptr<const PackageIDSequence> r(env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_STRINGIFY_EQUAL(join(indirect_iterator(r->begin()), indirect_iterator(r->end()), " | "),
                    "cat/pkg-1:0::repo | cat/pkg-2:0::repo | virtual/pkg-2::virtuals (virtual for cat/pkg-2:0::repo)");
        }
    } test_virtuals_repository;

    struct VirtualsRepositoryDuplicatesTest : TestCase
    {
        VirtualsRepositoryDuplicatesTest() : TestCase("virtuals repository duplicates") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<VirtualsRepository> virtuals(new VirtualsRepository(&env));
            tr1::shared_ptr<FakeRepository> repo1(new FakeRepository(&env, RepositoryName("repo1")));
            tr1::shared_ptr<FakeRepository> repo2(new FakeRepository(&env, RepositoryName("repo2")));
            tr1::shared_ptr<FakeInstalledRepository> installed(new FakeInstalledRepository(&env, RepositoryName("installed")));

            env.package_database()->add_repository(2, virtuals);
            env.package_database()->add_repository(3, repo1);
            env.package_database()->add_repository(4, repo2);
            env.package_database()->add_repository(5, installed);

            repo1->add_version("cat", "pkg", "1")->provide_key()->set_from_string("virtual/pkg");
            repo1->add_version("cat", "pkg", "2")->provide_key()->set_from_string("virtual/pkg");
            repo1->add_virtual_package(QualifiedPackageName("virtual/pkg"), make_shared_ptr(new PackageDepSpec(">=cat/pkg-2", pds_pm_permissive)));
            repo1->add_virtual_package(QualifiedPackageName("virtual/foo"), make_shared_ptr(new PackageDepSpec(">=cat/pkg-2", pds_pm_permissive)));

            repo2->add_virtual_package(QualifiedPackageName("virtual/pkg"), make_shared_ptr(new PackageDepSpec(">=cat/pkg-2", pds_pm_permissive)));
            repo2->add_virtual_package(QualifiedPackageName("virtual/foo"), make_shared_ptr(new PackageDepSpec("<=cat/pkg-1", pds_pm_permissive)));

            TEST_CHECK(virtuals->has_category_named(CategoryNamePart("virtual")));
            TEST_CHECK(virtuals->has_package_named(QualifiedPackageName("virtual/pkg")));

            tr1::shared_ptr<const PackageIDSequence> r(env.package_database()->query(query::All(), qo_order_by_version));
            TEST_CHECK_STRINGIFY_EQUAL(join(indirect_iterator(r->begin()), indirect_iterator(r->end()), " | "),
                    "cat/pkg-1:0::repo1 | cat/pkg-2:0::repo1 | "
                    "virtual/foo-1::virtuals (virtual for cat/pkg-1:0::repo1) | "
                    "virtual/foo-2::virtuals (virtual for cat/pkg-2:0::repo1) | "
                    "virtual/pkg-2::virtuals (virtual for cat/pkg-2:0::repo1)");
        }
    } test_virtuals_repository_duplicates;
}

