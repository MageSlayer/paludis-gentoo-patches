/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009 Ciaran McCreesh
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

#include <paludis/repositories/e/fix_locked_dependencies.hh>
#include <paludis/repositories/e/dep_parser.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/package_database.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;
using namespace paludis::erepository;

namespace test_cases
{
    struct FixLockedDependenciesTest : TestCase
    {
        FixLockedDependenciesTest() : TestCase("fix locked dependencies") { }

        void run()
        {
            TestEnvironment env;
            const std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(make_named_values<FakeRepositoryParams>(
                            value_for<n::environment>(&env),
                            value_for<n::name>(RepositoryName("repo"))
                            )));
            env.package_database()->add_repository(1, repo);
            std::tr1::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);
            installed_repo->add_version("cat", "installed", "1")->set_slot(SlotName("monkey"));

            std::tr1::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"));

            std::tr1::shared_ptr<const DependencySpecTree> bb(parse_depend(
                        "|| ( foo/bar ( bar/baz oink/squeak ) ) blah/blah", &env, id, *eapi)),
                aa(fix_locked_dependencies(&env, *eapi, id, bb));

            StringifyFormatter ff;
            DepSpecPrettyPrinter
                a(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false),
                b(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            aa->root()->accept(a);
            bb->root()->accept(b);

            TEST_CHECK_STRINGIFY_EQUAL(a, b);

            std::tr1::shared_ptr<const DependencySpecTree> cc(parse_depend(
                        "foo/bar:= cat/installed:= >=cat/installed-1.2:= <=cat/installed-1.2:=", &env, id, *eapi)),
                dd(fix_locked_dependencies(&env, *eapi, id, cc));

            DepSpecPrettyPrinter
                c(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false),
                d(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false, false);
            cc->root()->accept(c);
            dd->root()->accept(d);

            TEST_CHECK_STRINGIFY_EQUAL(c, "foo/bar:= cat/installed:= >=cat/installed-1.2:= <=cat/installed-1.2:=");
            TEST_CHECK_STRINGIFY_EQUAL(d, "foo/bar:= cat/installed:=monkey >=cat/installed-1.2:= <=cat/installed-1.2:=monkey");
        }
    } test_fix_locked_dependencies;
}

