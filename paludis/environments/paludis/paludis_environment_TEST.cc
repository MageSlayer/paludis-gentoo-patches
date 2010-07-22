/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010 Ciaran McCreesh
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

#include "paludis_environment.hh"
#include "paludis_config.hh"
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>
#include <test/test_runner.hh>
#include <test/test_framework.hh>
#include <cstdlib>

using namespace paludis;
using namespace test;

namespace
{
    bool get_use(const std::string & f, const std::shared_ptr<const PackageID> & id)
    {
        const std::shared_ptr<const ChoiceValue> v(id->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(f)));
        if (! v)
            return false;
        return v->enabled();
    }
}

namespace test_cases
{
    struct TestPaludisEnvironmentUse : TestCase
    {
        TestPaludisEnvironmentUse() : TestCase("use") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home1").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::shared_ptr<Environment> env(new PaludisEnvironment(""));
            const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(get_use("foo", one));
            TEST_CHECK(! get_use("foofoo", one));
            TEST_CHECK(get_use("moo", one));
            TEST_CHECK(get_use("quoted-name", one));

            TEST_CHECK(get_use("more_exp_one", one));
            TEST_CHECK(get_use("exp_two", one));
            TEST_CHECK(get_use("exp_one", one));
            TEST_CHECK(get_use("third_exp_one", one));
            TEST_CHECK(! get_use("third_exp_two", one));

            TEST_CHECK(get_use("third_exp_one", three));
            TEST_CHECK(get_use("third_exp_two", three));
        }
    } paludis_environment_use_test;

    struct TestPaludisEnvironmentKnownUse : TestCase
    {
        TestPaludisEnvironmentKnownUse() : TestCase("known use") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home5").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const std::shared_ptr<const PackageID> id1(*(*env)[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            std::shared_ptr<const Choice> foo_cards;
            for (Choices::ConstIterator c(id1->choices_key()->value()->begin()), c_end(id1->choices_key()->value()->end()) ;
                    c != c_end ; ++c)
                if ((*c)->raw_name() == "FOO_CARDS")
                    foo_cards = *c;
            if (! foo_cards)
                throw InternalError(PALUDIS_HERE, "oops");
            std::shared_ptr<const Set<UnprefixedChoiceName> > k1(env->known_choice_value_names(id1, foo_cards));
            TEST_CHECK_EQUAL(join(k1->begin(), k1->end(), " "), "one three two");
        }
    } paludis_environment_use_test_known;

    struct TestPaludisEnvironmentUseMinusStar : TestCase
    {
        TestPaludisEnvironmentUseMinusStar() : TestCase("use -*") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home2").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(get_use("foo", one));
            TEST_CHECK(! get_use("foofoo", one));
            TEST_CHECK(! get_use("moo", one));

            TEST_CHECK(get_use("more_exp_one", one));
            TEST_CHECK(get_use("exp_two", one));
            TEST_CHECK(! get_use("exp_one", one));
            TEST_CHECK(get_use("third_exp_one", one));
            TEST_CHECK(! get_use("third_exp_two", one));

            TEST_CHECK(! get_use("third_exp_one", three));
            TEST_CHECK(get_use("third_exp_two", three));
        }
    } paludis_environment_use_test_minus_star;

    struct TestPaludisEnvironmentUseMinusPartialStar : TestCase
    {
        TestPaludisEnvironmentUseMinusPartialStar() : TestCase("use -* partial") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home3").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::shared_ptr<Environment> env(new PaludisEnvironment(""));

            const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());
            const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                        generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                                    env.get(), UserPackageDepSpecOptions())), MatchPackageOptions()))]->begin());

            TEST_CHECK(get_use("foo", one));
            TEST_CHECK(! get_use("foofoo", one));
            TEST_CHECK(get_use("moo", one));

            TEST_CHECK(get_use("more_exp_one", one));
            TEST_CHECK(get_use("exp_two", one));
            TEST_CHECK(! get_use("exp_one", one));
            TEST_CHECK(get_use("third_exp_one", one));
            TEST_CHECK(! get_use("third_exp_two", one));

            TEST_CHECK(! get_use("third_exp_one", three));
            TEST_CHECK(get_use("third_exp_two", three));
        }
    } paludis_environment_use_test_minus_star_partial;

    struct TestPaludisEnvironmentRepositories : TestCase
    {
        TestPaludisEnvironmentRepositories() : TestCase("repositories") { }

        void run()
        {
            setenv("PALUDIS_HOME", stringify(FSEntry::cwd() / "paludis_environment_TEST_dir" / "home4").c_str(), 1);
            unsetenv("PALUDIS_SKIP_CONFIG");

            std::shared_ptr<Environment> env(new PaludisEnvironment(""));

            TEST_CHECK(bool(env->package_database()->fetch_repository(RepositoryName("first"))));
            TEST_CHECK(bool(env->package_database()->fetch_repository(RepositoryName("second"))));
            TEST_CHECK(bool(env->package_database()->fetch_repository(RepositoryName("third"))));
            TEST_CHECK(bool(env->package_database()->fetch_repository(RepositoryName("fourth"))));
            TEST_CHECK(bool(env->package_database()->fetch_repository(RepositoryName("fifth"))));

            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("first"), RepositoryName("second")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("third")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("third")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("fifth")));
            TEST_CHECK(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("fifth")));
        }
    } paludis_environment_repositories;
}

