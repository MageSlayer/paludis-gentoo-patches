/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010 Ciaran McCreesh
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

#include <paludis/repositories/e/e_repository.hh>
#include <paludis/repositories/e/e_repository_exceptions.hh>
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <functional>
#include <set>
#include <string>

#include "config.h"

using namespace test;
using namespace paludis;

namespace
{
    void cannot_uninstall(const std::shared_ptr<const PackageID> & id, const UninstallActionOptions &)
    {
        if (id)
            throw InternalError(PALUDIS_HERE, "cannot uninstall");
    }

    std::shared_ptr<OutputManager> make_standard_output_manager(const Action &)
    {
        return std::make_shared<StandardOutputManager>();
    }

    std::string from_keys(const std::shared_ptr<const Map<std::string, std::string> > & m,
            const std::string & k)
    {
        Map<std::string, std::string>::ConstIterator mm(m->find(k));
        if (m->end() == mm)
            return "";
        else
            return mm->second;
    }

    WantPhase want_all_phases(const std::string &)
    {
        return wp_yes;
    }
}

namespace test_cases
{
    struct ERepositoryDependenciesRewriterTest : TestCase
    {
        ERepositoryDependenciesRewriterTest() : TestCase("dependencies_rewriter") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "e_repository_TEST_dependencies_rewriter_dir" / "repo"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "e_repository_TEST_dependencies_rewriter_dir" / "repo/profiles/profile"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dependencies_rewriter_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions())), MatchPackageOptions()))]->last());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->build_dependencies_key()));
            id->build_dependencies_key()->value()->root()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "( cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: post: )");

            erepository::DepSpecPrettyPrinter pr(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->run_dependencies_key()));
            id->run_dependencies_key()->value()->root()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "( cat/pkg1 build: build+run: cat/pkg3 suggestion: post: )");

            erepository::DepSpecPrettyPrinter pp(0, std::shared_ptr<const PackageID>(), ff, 0, false, false);
            TEST_CHECK(bool(id->post_dependencies_key()));
            id->post_dependencies_key()->value()->root()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "( build: build+run: suggestion: cat/pkg4 post: cat/pkg5 )");
        }
    } test_e_repository_dependencies_rewriter;
}

