/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>

#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/make_named_values.hh>
#include <paludis/util/stringify.hh>

#include <paludis/unformatted_pretty_printer.hh>

#include <gtest/gtest.h>

using namespace paludis;
using namespace paludis::erepository;

TEST(FixLockedDependencies, Works)
{
    TestEnvironment env;
    const std::shared_ptr<FakeRepository> repo(std::make_shared<FakeRepository>(make_named_values<FakeRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("repo")
                    )));
    env.add_repository(1, repo);
    std::shared_ptr<const PackageID> id(repo->add_version("cat", "pkg", "1"));

    std::shared_ptr<FakeInstalledRepository> installed_repo(std::make_shared<FakeInstalledRepository>(
                make_named_values<FakeInstalledRepositoryParams>(
                    n::environment() = &env,
                    n::name() = RepositoryName("installed"),
                    n::suitable_destination() = true,
                    n::supports_uninstall() = true
                    )));
    env.add_repository(2, installed_repo);
    installed_repo->add_version("cat", "installed", "1")->set_slot(SlotName("monkey"));

    std::shared_ptr<const EAPI> eapi(EAPIData::get_instance()->eapi_from_string("paludis-1"));

    std::shared_ptr<const DependencySpecTree> bb(parse_depend(
                "|| ( foo/bar ( bar/baz oink/squeak ) ) blah/blah", &env, *eapi, false));
    std::shared_ptr<const DependencySpecTree> aa(fix_locked_dependencies(&env, *eapi, id, bb));

    UnformattedPrettyPrinter ff;
    SpecTreePrettyPrinter a(ff, {});
    SpecTreePrettyPrinter b(ff, {});
    aa->top()->accept(a);
    bb->top()->accept(b);

    EXPECT_EQ(stringify(b), stringify(a));

    std::shared_ptr<const DependencySpecTree> cc(parse_depend(
                "foo/bar:= cat/installed:= >=cat/installed-1.2:= <=cat/installed-1.2:=", &env, *eapi, false));
    std::shared_ptr<const DependencySpecTree> dd(fix_locked_dependencies(&env, *eapi, id, cc));

    SpecTreePrettyPrinter c(ff, {});
    SpecTreePrettyPrinter d(ff, {});
    cc->top()->accept(c);
    dd->top()->accept(d);

    EXPECT_EQ("foo/bar:= cat/installed:= >=cat/installed-1.2:= <=cat/installed-1.2:=", stringify(c));
    EXPECT_EQ("foo/bar:= cat/installed:=monkey >=cat/installed-1.2:= <=cat/installed-1.2:=monkey", stringify(d));
}

