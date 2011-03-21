/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2010, 2011 Ciaran McCreesh
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

#include <paludis/environments/paludis/paludis_environment.hh>
#include <paludis/environments/paludis/paludis_config.hh>

#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/options.hh>
#include <paludis/util/make_null_shared_ptr.hh>

#include <paludis/package_id.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/metadata_key.hh>
#include <paludis/choice.hh>

#include <cstdlib>
#include <gtest/gtest.h>

using namespace paludis;

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

TEST(PaludisEnvironment, Use)
{
    setenv("PALUDIS_HOME", stringify(FSPath::cwd() / "paludis_environment_TEST_dir" / "home1").c_str(), 1);
    unsetenv("PALUDIS_SKIP_CONFIG");

    std::shared_ptr<Environment> env(std::make_shared<PaludisEnvironment>(""));
    const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());
    const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(get_use("foo", one));
    EXPECT_TRUE(! get_use("foofoo", one));
    EXPECT_TRUE(get_use("moo", one));
    EXPECT_TRUE(get_use("quoted-name", one));

    EXPECT_TRUE(get_use("more_exp_one", one));
    EXPECT_TRUE(get_use("exp_two", one));
    EXPECT_TRUE(get_use("exp_one", one));
    EXPECT_TRUE(get_use("third_exp_one", one));
    EXPECT_TRUE(! get_use("third_exp_two", one));

    EXPECT_TRUE(get_use("third_exp_one", three));
    EXPECT_TRUE(get_use("third_exp_two", three));
}

TEST(PaludisEnvironment, KnownUse)
{
    setenv("PALUDIS_HOME", stringify(FSPath::cwd() / "paludis_environment_TEST_dir" / "home5").c_str(), 1);
    unsetenv("PALUDIS_SKIP_CONFIG");

    std::shared_ptr<Environment> env(std::make_shared<PaludisEnvironment>(""));

    const std::shared_ptr<const PackageID> id1(*(*env)[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());
    std::shared_ptr<const Choice> foo_cards;
    for (Choices::ConstIterator c(id1->choices_key()->value()->begin()), c_end(id1->choices_key()->value()->end()) ;
            c != c_end ; ++c)
        if ((*c)->raw_name() == "FOO_CARDS")
            foo_cards = *c;
    if (! foo_cards)
        throw InternalError(PALUDIS_HERE, "oops");
    std::shared_ptr<const Set<UnprefixedChoiceName> > k1(env->known_choice_value_names(id1, foo_cards));
    EXPECT_EQ("one three two", join(k1->begin(), k1->end(), " "));
}

TEST(PaludisEnvironment, UseMinusStar)
{
    setenv("PALUDIS_HOME", stringify(FSPath::cwd() / "paludis_environment_TEST_dir" / "home2").c_str(), 1);
    unsetenv("PALUDIS_SKIP_CONFIG");

    std::shared_ptr<Environment> env(std::make_shared<PaludisEnvironment>(""));

    const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());
    const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(get_use("foo", one));
    EXPECT_TRUE(! get_use("foofoo", one));
    EXPECT_TRUE(! get_use("moo", one));

    EXPECT_TRUE(get_use("more_exp_one", one));
    EXPECT_TRUE(get_use("exp_two", one));
    EXPECT_TRUE(! get_use("exp_one", one));
    EXPECT_TRUE(get_use("third_exp_one", one));
    EXPECT_TRUE(! get_use("third_exp_two", one));

    EXPECT_TRUE(! get_use("third_exp_one", three));
    EXPECT_TRUE(get_use("third_exp_two", three));
}

TEST(PaludisEnvironment, UseMinusPartialStar)
{
    setenv("PALUDIS_HOME", stringify(FSPath::cwd() / "paludis_environment_TEST_dir" / "home3").c_str(), 1);
    unsetenv("PALUDIS_SKIP_CONFIG");

    std::shared_ptr<Environment> env(std::make_shared<PaludisEnvironment>(""));

    const std::shared_ptr<const PackageID> one(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());
    const std::shared_ptr<const PackageID> three(*(*env)[selection::RequireExactlyOne(
                generator::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-3",
                            env.get(), { })), make_null_shared_ptr(), { }))]->begin());

    EXPECT_TRUE(get_use("foo", one));
    EXPECT_TRUE(! get_use("foofoo", one));
    EXPECT_TRUE(get_use("moo", one));

    EXPECT_TRUE(get_use("more_exp_one", one));
    EXPECT_TRUE(get_use("exp_two", one));
    EXPECT_TRUE(! get_use("exp_one", one));
    EXPECT_TRUE(get_use("third_exp_one", one));
    EXPECT_TRUE(! get_use("third_exp_two", one));

    EXPECT_TRUE(! get_use("third_exp_one", three));
    EXPECT_TRUE(get_use("third_exp_two", three));
}

TEST(PaludisEnvironment, Repositories)
{
    setenv("PALUDIS_HOME", stringify(FSPath::cwd() / "paludis_environment_TEST_dir" / "home4").c_str(), 1);
    unsetenv("PALUDIS_SKIP_CONFIG");

    std::shared_ptr<Environment> env(std::make_shared<PaludisEnvironment>(""));

    EXPECT_TRUE(bool(env->package_database()->fetch_repository(RepositoryName("first"))));
    EXPECT_TRUE(bool(env->package_database()->fetch_repository(RepositoryName("second"))));
    EXPECT_TRUE(bool(env->package_database()->fetch_repository(RepositoryName("third"))));
    EXPECT_TRUE(bool(env->package_database()->fetch_repository(RepositoryName("fourth"))));
    EXPECT_TRUE(bool(env->package_database()->fetch_repository(RepositoryName("fifth"))));

    EXPECT_TRUE(env->package_database()->more_important_than(RepositoryName("first"), RepositoryName("second")));
    EXPECT_TRUE(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("third")));
    EXPECT_TRUE(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("third")));
    EXPECT_TRUE(env->package_database()->more_important_than(RepositoryName("fourth"), RepositoryName("fifth")));
    EXPECT_TRUE(env->package_database()->more_important_than(RepositoryName("second"), RepositoryName("fifth")));
}

