/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
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
#include <paludis/repositories/e/spec_tree_pretty_printer.hh>

#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/environments/test/test_environment.hh>

#include <paludis/util/system.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/stringify.hh>

#include <paludis/standard_output_manager.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <paludis/repository_factory.hh>
#include <paludis/choice.hh>
#include <paludis/unformatted_pretty_printer.hh>

#include <paludis/util/indirect_iterator-impl.hh>

#include <algorithm>
#include <functional>
#include <set>
#include <string>

#include "config.h"

#include <gtest/gtest.h>

using namespace paludis;

namespace
{
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

    std::string contents(const std::string & filename)
    {
        SafeIFStream s(FSPath(filename).realpath());
        return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
    }
}

TEST(ERepository, RepoName)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    EXPECT_EQ("test-repo-1", stringify(repo->name()));
}

TEST(ERepository, NoRepoName)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo2"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo2/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    EXPECT_EQ("x-repo2", stringify(repo->name()));
}

TEST(ERepository, EmptyRepoName)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo3"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo3/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    EXPECT_EQ("x-repo3", stringify(repo->name()));
}

TEST(ERepository, HasCategoryNamed)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        EXPECT_TRUE(repo->has_category_named(CategoryNamePart("cat-one"), { }));
        EXPECT_TRUE(repo->has_category_named(CategoryNamePart("cat-two"), { }));
        EXPECT_TRUE(repo->has_category_named(CategoryNamePart("cat-three"), { }));
        EXPECT_TRUE(! repo->has_category_named(CategoryNamePart("cat-four"), { }));
    }
}

TEST(ERepository, CategoryNames)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        std::shared_ptr<const CategoryNamePartSet> c(repo->category_names({ }));
        EXPECT_TRUE(c->end() != c->find(CategoryNamePart("cat-one")));
        EXPECT_TRUE(c->end() != c->find(CategoryNamePart("cat-two")));
        EXPECT_TRUE(c->end() != c->find(CategoryNamePart("cat-three")));
        EXPECT_TRUE(c->end() == c->find(CategoryNamePart("cat-four")));
        EXPECT_EQ(3, std::distance(c->begin(), c->end()));
    }
}

TEST(ERepository, HasPackageNamed)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither"), { }));
    }
}

TEST(ERepository, HasPackageNamedCached)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    repo->package_names(CategoryNamePart("cat-one"), { });
    repo->package_names(CategoryNamePart("cat-two"), { });
    repo->package_names(CategoryNamePart("cat-three"), { });

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both"), { }));
        EXPECT_TRUE(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both"), { }));
        EXPECT_TRUE(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither"), { }));
    }
}

TEST(ERepository, PackageNames)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    std::shared_ptr<const QualifiedPackageNameSet> names;

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        names = repo->package_names(CategoryNamePart("cat-one"), { });
        EXPECT_TRUE(! names->empty());
        EXPECT_TRUE(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
        EXPECT_TRUE(names->end() != names->find(QualifiedPackageName("cat-one/pkg-both")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-one/pkg-two")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-one/pkg-neither")));
        EXPECT_EQ(2, std::distance(names->begin(), names->end()));

        names = repo->package_names(CategoryNamePart("cat-two"), { });
        EXPECT_TRUE(! names->empty());
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-two/pkg-one")));
        EXPECT_TRUE(names->end() != names->find(QualifiedPackageName("cat-two/pkg-both")));
        EXPECT_TRUE(names->end() != names->find(QualifiedPackageName("cat-two/pkg-two")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-two/pkg-neither")));
        EXPECT_EQ(2, std::distance(names->begin(), names->end()));

        names = repo->package_names(CategoryNamePart("cat-three"), { });
        EXPECT_TRUE(names->empty());
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-three/pkg-one")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-three/pkg-both")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-three/pkg-two")));
        EXPECT_TRUE(names->end() == names->find(QualifiedPackageName("cat-three/pkg-neither")));
        EXPECT_EQ(0, std::distance(names->begin(), names->end()));
    }
}

TEST(ERepository, BadPackageNames)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo5"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo5/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));

    std::shared_ptr<const QualifiedPackageNameSet> names;

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        names = repo->package_names(CategoryNamePart("cat-one"), { });
        EXPECT_TRUE(! names->empty());
        EXPECT_TRUE(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
        EXPECT_EQ(1, std::distance(names->begin(), names->end()));
    }
}

TEST(ERepository, PackageID)
{
    using namespace std::placeholders;

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        std::shared_ptr<const PackageIDSequence> versions;

        versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"), { });
        EXPECT_TRUE(! versions->empty());
        EXPECT_EQ(2, std::distance(versions->begin(), versions->end()));
        EXPECT_TRUE(indirect_iterator(versions->end()) != std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1", VersionSpecOptions()))));
        EXPECT_TRUE(indirect_iterator(versions->end()) != std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1", VersionSpecOptions()))));
        EXPECT_TRUE(indirect_iterator(versions->end()) == std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("2", VersionSpecOptions()))));

        versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"), { });
        EXPECT_TRUE(versions->empty());
        EXPECT_EQ(0, std::distance(versions->begin(), versions->end()));
    }
}

TEST(ERepository, DuffVersions)
{
    using namespace std::placeholders;

    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo8"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo8/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        std::shared_ptr<const PackageIDSequence> versions;

        versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"), { });
        EXPECT_TRUE(! versions->empty());
        EXPECT_EQ(2, std::distance(versions->begin(), versions->end()));
        EXPECT_TRUE(indirect_iterator(versions->end()) != std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1", VersionSpecOptions()))));
        EXPECT_TRUE(indirect_iterator(versions->end()) != std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1", VersionSpecOptions()))));
        EXPECT_TRUE(indirect_iterator(versions->end()) == std::find_if(
                    indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                    std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("2", VersionSpecOptions()))));

        versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"), { });
        EXPECT_TRUE(versions->empty());
        EXPECT_EQ(0, std::distance(versions->begin(), versions->end()));
    }
}

TEST(ERepository, MetadataUncached)
{
    for (int opass = 1 ; opass <= 3 ; ++opass)
    {
        TestEnvironment env;
        std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
        keys->insert("format", "e");
        keys->insert("names_cache", "/var/empty");
        keys->insert("write_cache", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/metadata/cache"));
        keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7"));
        keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
        keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
        std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys, std::placeholders::_1)));
        env.add_repository(1, repo);

        for (int pass = 1 ; pass <= 3 ; ++pass)
        {
            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                    &env, { })), nullptr, { }))]->begin());

            EXPECT_TRUE(id1->end_metadata() != id1->find_metadata("EAPI"));
            EXPECT_TRUE(visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI")));
            EXPECT_EQ("0", visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI"))->parse_value());
            ASSERT_TRUE(bool(id1->short_description_key()));
            EXPECT_EQ("The Description", id1->short_description_key()->parse_value());
            UnformattedPrettyPrinter ff;
            erepository::SpecTreePrettyPrinter pd(ff, { });
            ASSERT_TRUE(bool(id1->build_dependencies_key()));
            id1->build_dependencies_key()->parse_value()->top()->accept(pd);
            EXPECT_EQ("foo/bar", stringify(pd));
            erepository::SpecTreePrettyPrinter pr(ff, { });
            ASSERT_TRUE(bool(id1->run_dependencies_key()));
            id1->run_dependencies_key()->parse_value()->top()->accept(pr);
            EXPECT_EQ("foo/bar", stringify(pr));

            const std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                    &env, { })), nullptr, { }))]->begin());

            ASSERT_TRUE(id2->end_metadata() != id2->find_metadata("EAPI"));
            ASSERT_TRUE(bool(id2->short_description_key()));
            EXPECT_EQ("dquote \" squote ' backslash \\ dollar $", id2->short_description_key()->parse_value());
            erepository::SpecTreePrettyPrinter pd2(ff, { });
            ASSERT_TRUE(bool(id2->build_dependencies_key()));
            id2->build_dependencies_key()->parse_value()->top()->accept(pd2);
            EXPECT_EQ("foo/bar bar/baz", stringify(pd2));
            erepository::SpecTreePrettyPrinter pr2(ff, { });
            ASSERT_TRUE(bool(id2->run_dependencies_key()));
            id2->run_dependencies_key()->parse_value()->top()->accept(pr2);
            EXPECT_EQ("foo/bar", stringify(pr2));

            const std::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-3",
                                    &env, { })), nullptr, { }))]->begin());

            ASSERT_TRUE(id3->end_metadata() != id3->find_metadata("EAPI"));
            ASSERT_TRUE(bool(id3->short_description_key()));
            EXPECT_EQ("This is the short description", id3->short_description_key()->parse_value());
            ASSERT_TRUE(bool(id3->long_description_key()));
            EXPECT_EQ("This is the long description", id3->long_description_key()->parse_value());
        }
    }
}

TEST(ERepository, MetadataUnparsable)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                &env, { })), nullptr, { }))]->begin());

        ASSERT_TRUE(id1->end_metadata() != id1->find_metadata("EAPI"));
        EXPECT_EQ("UNKNOWN", std::static_pointer_cast<const erepository::ERepositoryID>(id1)->eapi()->name());
        ASSERT_TRUE(! id1->short_description_key());
    }
}

namespace
{
    struct ERepositoryQueryUseTest :
        testing::Test
    {
        void test_choice(const std::shared_ptr<const PackageID> & p, const std::string & n, bool enabled, bool enabled_by_default, bool locked, const std::string & u = "")
        {
            std::shared_ptr<const ChoiceValue> choice(p->choices_key()->parse_value()->find_by_name_with_prefix(ChoiceNameWithPrefix(n)));
            ASSERT_TRUE(bool(choice)) << "checking package '" << *p << "' choice '" << n << "'";
            EXPECT_EQ(choice->unprefixed_name(), UnprefixedChoiceName(u.empty() ? n : u)) << "checking package '" << *p << "' choice '" << n << "'";
            EXPECT_EQ(enabled, choice->enabled()) << "checking package '" << *p << "' choice '" << n << "'";
            EXPECT_EQ(enabled_by_default, choice->enabled_by_default()) << "checking package '" << *p << "' choice '" << n << "'";
            EXPECT_EQ(locked, choice->locked()) << "checking package '" << *p << "' choice '" << n << "'";
        }
    };
}

TEST_F(ERepositoryQueryUseTest, QueryUse)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9/profiles/child"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys, std::placeholders::_1))));
    env.add_repository(1, repo);

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        const std::shared_ptr<const PackageID> p1(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                &env, { })), nullptr, { }))]->begin());
        const std::shared_ptr<const PackageID> p2(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat-two/pkg-two-1",
                                &env, { })), nullptr, { }))]->begin());
        const std::shared_ptr<const PackageID> p4(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                &env, { })), nullptr, { }))]->begin());

        test_choice(p1, "flag1",     true,  true,  false);
        test_choice(p1, "flag2",     false, false, true);
        test_choice(p1, "flag3",     true,  true,  false);
        test_choice(p1, "flag4",     true,  true,  true);
        test_choice(p1, "flag5",     false, false, false);
        test_choice(p1, "enabled",   true,  false, false);
        test_choice(p1, "disabled",  false, true,  false);
        test_choice(p1, "enabled2",  true,  false, false);
        test_choice(p1, "disabled2", false, true,  false);
        test_choice(p1, "enabled3",  false, false, true);
        test_choice(p1, "disabled3", true,  true,  true);

        test_choice(p2, "flag1", true,  true,  false);
        test_choice(p2, "flag2", false, false, true);
        test_choice(p2, "flag3", false, false, true);
        test_choice(p2, "flag4", true,  true,  true);
        test_choice(p2, "flag5", true,  true,  true);
        test_choice(p2, "flag6", true,  true,  false);

        test_choice(p4, "flag1", true,  true,  false);
        test_choice(p4, "flag2", false, false, true);
        test_choice(p4, "flag3", false, false, true);
        test_choice(p4, "flag4", true,  true,  true);
        test_choice(p4, "flag5", true,  true,  false);
        test_choice(p4, "flag6", false, false, false);

        test_choice(p1, "test",  true,  true,  true);
        test_choice(p1, "test2", false, false, true);

        test_choice(p1, "not_in_iuse_ennobled", true, true, false, "ennobled");
        test_choice(p1, "not_in_iuse_masked", false, false, true, "masked");
        test_choice(p1, "not_in_iuse_forced", true, true, true, "forced");
        test_choice(p1, "not_in_iuse_ennobled_package", true, true, false, "ennobled_package");
        test_choice(p1, "not_in_iuse_disabled_package", false, false, false, "disabled_package");
        test_choice(p1, "not_in_iuse_masked_package", false, false, true, "masked_package");
        test_choice(p1, "not_in_iuse_forced_package", false, false, false, "forced_package");

        test_choice(p2, "not_in_iuse_ennobled", false, false, false, "ennobled");
        test_choice(p2, "not_in_iuse_masked", false, false, true, "masked");
        test_choice(p2, "not_in_iuse_forced", true, true, true, "forced");
        test_choice(p2, "not_in_iuse_ennobled_package", false, false, false, "ennobled_package");
        test_choice(p2, "not_in_iuse_disabled_package", false, false, false, "disabled_package");
        test_choice(p2, "not_in_iuse_masked_package", false, false, false, "masked_package");
        test_choice(p2, "not_in_iuse_forced_package", true, true, true, "forced_package");
    }
}

TEST_F(ERepositoryQueryUseTest, UseStableMaskForce)
{
    bool accept_unstable(false);
    do
    {
        TestEnvironment env(accept_unstable);

        std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
        keys->insert("format", "e");
        keys->insert("names_cache", "/var/empty");
        keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9a"));
        keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9a/profiles/eapi5/child"));
        keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
        std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1))));
        env.add_repository(1, repo);

        for (int pass = 1 ; pass <= 2 ; ++pass)
        {
            const std::shared_ptr<const PackageID> stable1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/stable-1",
                                    &env, { })), nullptr, { }))]->begin());
            const std::shared_ptr<const PackageID> stable1r1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/stable-1-r1",
                                    &env, { })), nullptr, { }))]->begin());
            const std::shared_ptr<const PackageID> stable2(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/stable-2",
                                    &env, { })), nullptr, { }))]->begin());
            const std::shared_ptr<const PackageID> unstable1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/unstable-1",
                                    &env, { })), nullptr, { }))]->begin());
            const std::shared_ptr<const PackageID> missing1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/missing-1",
                                    &env, { })), nullptr, { }))]->begin());

            test_choice(stable1, "notstmask",     false, false, false);
            test_choice(stable1, "notpkgstmask",  false, false, false);
            test_choice(stable1, "notstforce",    false, false, false);
            test_choice(stable1, "notpkgstforce", false, false, false);

            test_choice(stable1, "stmask", false, false, ! accept_unstable);
            test_choice(stable1, "pkgstmask", false, false, ! accept_unstable);
            test_choice(stable1, "stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1, "pkgstforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);

            test_choice(stable1, "mask-stunmask", false, false, accept_unstable);
            test_choice(stable1, "unmask-stmask", false, false, ! accept_unstable);
            test_choice(stable1, "stmask-pkgunmask", false, false, false);
            test_choice(stable1, "stunmask-pkgmask", false, false, true);
            test_choice(stable1, "pkgmask-pkgstunmask", false, false, accept_unstable);
            test_choice(stable1, "pkgunmask-pkgstmask", false, false, ! accept_unstable);
            test_choice(stable1, "pkgstmask-chunmask", false, false, false);
            test_choice(stable1, "pkgstunmask-chmask", false, false, true);
            test_choice(stable1, "pkgstmask-chpkgstunmask", false, false, false);
            test_choice(stable1, "pkgstunmask-chpkgstmask", false, false, ! accept_unstable);

            test_choice(stable1, "force-stunforce", accept_unstable, accept_unstable, accept_unstable);
            test_choice(stable1, "unforce-stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1, "stforce-pkgunforce", false, false, false);
            test_choice(stable1, "stunforce-pkgforce", true, true, true);
            test_choice(stable1, "pkgforce-pkgstunforce", accept_unstable, accept_unstable, accept_unstable);
            test_choice(stable1, "pkgunforce-pkgstforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1, "pkgstforce-chunforce", false, false, false);
            test_choice(stable1, "pkgstunforce-chforce", true, true, true);
            test_choice(stable1, "pkgstforce-chpkgstunforce", false, false, false);
            test_choice(stable1, "pkgstunforce-chpkgstforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);

            test_choice(stable1r1, "notstmask",     false, false, false);
            test_choice(stable1r1, "notpkgstmask",  false, false, false);
            test_choice(stable1r1, "notstforce",    false, false, false);
            test_choice(stable1r1, "notpkgstforce", false, false, false);

            test_choice(stable1r1, "stmask", false, false, ! accept_unstable);
            test_choice(stable1r1, "pkgstmask", false, false, false);
            test_choice(stable1r1, "stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1r1, "pkgstforce", false, false, false);

            test_choice(stable1r1, "mask-stunmask", false, false, accept_unstable);
            test_choice(stable1r1, "unmask-stmask", false, false, ! accept_unstable);
            test_choice(stable1r1, "stmask-pkgunmask", false, false, ! accept_unstable);
            test_choice(stable1r1, "stunmask-pkgmask", false, false, false);
            test_choice(stable1r1, "pkgmask-pkgstunmask", false, false, false);
            test_choice(stable1r1, "pkgunmask-pkgstmask", false, false, false);
            test_choice(stable1r1, "pkgstmask-chunmask", false, false, false);
            test_choice(stable1r1, "pkgstunmask-chmask", false, false, true);
            test_choice(stable1r1, "pkgstmask-chpkgstunmask", false, false, ! accept_unstable);
            test_choice(stable1r1, "pkgstunmask-chpkgstmask", false, false, false);

            test_choice(stable1r1, "force-stunforce", accept_unstable, accept_unstable, accept_unstable);
            test_choice(stable1r1, "unforce-stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1r1, "stforce-pkgunforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1r1, "stunforce-pkgforce", false, false, false);
            test_choice(stable1r1, "pkgforce-pkgstunforce", false, false, false);
            test_choice(stable1r1, "pkgunforce-pkgstforce", false, false, false);
            test_choice(stable1r1, "pkgstforce-chunforce", false, false, false);
            test_choice(stable1r1, "pkgstunforce-chforce", true, true, true);
            test_choice(stable1r1, "pkgstforce-chpkgstunforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable1r1, "pkgstunforce-chpkgstforce", false, false, false);

            test_choice(stable2, "notstmask",     false, false, false);
            test_choice(stable2, "notpkgstmask",  false, false, false);
            test_choice(stable2, "notstforce",    false, false, false);
            test_choice(stable2, "notpkgstforce", false, false, false);

            test_choice(stable2, "stmask", false, false, ! accept_unstable);
            test_choice(stable2, "pkgstmask", false, false, false);
            test_choice(stable2, "stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable2, "pkgstforce", false, false, false);

            test_choice(stable2, "mask-stunmask", false, false, accept_unstable);
            test_choice(stable2, "unmask-stmask", false, false, ! accept_unstable);
            test_choice(stable2, "stmask-pkgunmask", false, false, ! accept_unstable);
            test_choice(stable2, "stunmask-pkgmask", false, false, false);
            test_choice(stable2, "pkgmask-pkgstunmask", false, false, false);
            test_choice(stable2, "pkgunmask-pkgstmask", false, false, false);
            test_choice(stable2, "pkgstmask-chunmask", false, false, false);
            test_choice(stable2, "pkgstunmask-chmask", false, false, true);
            test_choice(stable2, "pkgstmask-chpkgstunmask", false, false, false);
            test_choice(stable2, "pkgstunmask-chpkgstmask", false, false, false);

            test_choice(stable2, "force-stunforce", accept_unstable, accept_unstable, accept_unstable);
            test_choice(stable2, "unforce-stforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable2, "stforce-pkgunforce", ! accept_unstable, ! accept_unstable, ! accept_unstable);
            test_choice(stable2, "stunforce-pkgforce", false, false, false);
            test_choice(stable2, "pkgforce-pkgstunforce", false, false, false);
            test_choice(stable2, "pkgunforce-pkgstforce", false, false, false);
            test_choice(stable2, "pkgstforce-chunforce", false, false, false);
            test_choice(stable2, "pkgstunforce-chforce", true, true, true);
            test_choice(stable2, "pkgstforce-chpkgstunforce", false, false, false);
            test_choice(stable2, "pkgstunforce-chpkgstforce", false, false, false);

            test_choice(unstable1, "notstmask",     false, false, false);
            test_choice(unstable1, "notpkgstmask",  false, false, false);
            test_choice(unstable1, "notstforce",    false, false, false);
            test_choice(unstable1, "notpkgstforce", false, false, false);

            test_choice(unstable1, "stmask", false, false, false);
            test_choice(unstable1, "pkgstmask", false, false, false);
            test_choice(unstable1, "stforce", false, false, false);
            test_choice(unstable1, "pkgstforce", false, false, false);

            test_choice(unstable1, "mask-stunmask", false, false, true);
            test_choice(unstable1, "unmask-stmask", false, false, false);
            test_choice(unstable1, "stmask-pkgunmask", false, false, false);
            test_choice(unstable1, "stunmask-pkgmask", false, false, false);
            test_choice(unstable1, "pkgmask-pkgstunmask", false, false, false);
            test_choice(unstable1, "pkgunmask-pkgstmask", false, false, false);
            test_choice(unstable1, "pkgstmask-chunmask", false, false, false);
            test_choice(unstable1, "pkgstunmask-chmask", false, false, true);
            test_choice(unstable1, "pkgstmask-chpkgstunmask", false, false, false);
            test_choice(unstable1, "pkgstunmask-chpkgstmask", false, false, false);

            test_choice(unstable1, "force-stunforce", true, true, true);
            test_choice(unstable1, "unforce-stforce", false, false, false);
            test_choice(unstable1, "stforce-pkgunforce", false, false, false);
            test_choice(unstable1, "stunforce-pkgforce", false, false, false);
            test_choice(unstable1, "pkgforce-pkgstunforce", false, false, false);
            test_choice(unstable1, "pkgunforce-pkgstforce", false, false, false);
            test_choice(unstable1, "pkgstforce-chunforce", false, false, false);
            test_choice(unstable1, "pkgstunforce-chforce", true, true, true);
            test_choice(unstable1, "pkgstforce-chpkgstunforce", false, false, false);
            test_choice(unstable1, "pkgstunforce-chpkgstforce", false, false, false);

            test_choice(missing1, "notstmask",     false, false, false);
            test_choice(missing1, "notpkgstmask",  false, false, false);
            test_choice(missing1, "notstforce",    false, false, false);
            test_choice(missing1, "notpkgstforce", false, false, false);

            test_choice(missing1, "stmask", false, false, false);
            test_choice(missing1, "pkgstmask", false, false, false);
            test_choice(missing1, "stforce", false, false, false);
            test_choice(missing1, "pkgstforce", false, false, false);

            test_choice(missing1, "mask-stunmask", false, false, true);
            test_choice(missing1, "unmask-stmask", false, false, false);
            test_choice(missing1, "stmask-pkgunmask", false, false, false);
            test_choice(missing1, "stunmask-pkgmask", false, false, false);
            test_choice(missing1, "pkgmask-pkgstunmask", false, false, false);
            test_choice(missing1, "pkgunmask-pkgstmask", false, false, false);
            test_choice(missing1, "pkgstmask-chunmask", false, false, false);
            test_choice(missing1, "pkgstunmask-chmask", false, false, true);
            test_choice(missing1, "pkgstmask-chpkgstunmask", false, false, false);
            test_choice(missing1, "pkgstunmask-chpkgstmask", false, false, false);

            test_choice(missing1, "force-stunforce", true, true, true);
            test_choice(missing1, "unforce-stforce", false, false, false);
            test_choice(missing1, "stforce-pkgunforce", false, false, false);
            test_choice(missing1, "stunforce-pkgforce", false, false, false);
            test_choice(missing1, "pkgforce-pkgstunforce", false, false, false);
            test_choice(missing1, "pkgunforce-pkgstforce", false, false, false);
            test_choice(missing1, "pkgstforce-chunforce", false, false, false);
            test_choice(missing1, "pkgstunforce-chforce", true, true, true);
            test_choice(missing1, "pkgstforce-chpkgstunforce", false, false, false);
            test_choice(missing1, "pkgstunforce-chpkgstforce", false, false, false);
        }

        accept_unstable = ! accept_unstable;
    }
    while (accept_unstable);
}

TEST(ERepository, Masks)
{
    TestEnvironment env;

    std::shared_ptr<Map<std::string, std::string> > keys18(std::make_shared<Map<std::string, std::string>>());
    keys18->insert("format", "e");
    keys18->insert("names_cache", "/var/empty");
    keys18->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo18"));
    keys18->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo18/profiles/profile"));
    keys18->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo18(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys18, std::placeholders::_1)));
    env.add_repository(1, repo18);

    std::shared_ptr<Map<std::string, std::string> > keys19(std::make_shared<Map<std::string, std::string>>());
    keys19->insert("format", "e");
    keys19->insert("names_cache", "/var/empty");
    keys19->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo19"));
    keys19->insert("master_repository", "test-repo-18");
    keys19->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo19(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys19, std::placeholders::_1)));
    env.add_repository(1, repo19);

    EXPECT_TRUE((*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-18",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE((*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-18",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-18",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-18",
                                &env, { })), nullptr, { }))]->begin())->masked());

    EXPECT_TRUE((*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-19",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-19",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE((*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-19",
                                &env, { })), nullptr, { }))]->begin())->masked());
    EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-19",
                                &env, { })), nullptr, { }))]->begin())->masked());
}

TEST(ERepository, ProfileMasks)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo10"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo10/profiles/profile/subprofile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    for (int pass = 1 ; pass <= 2 ; ++pass)
    {
        EXPECT_TRUE((*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/masked-0",
                                    &env, { })), nullptr, { }))]->begin())->masked());
        EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                    &env, { })), nullptr, { }))]->begin())->masked());
        EXPECT_TRUE(! (*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/not_masked-0",
                                    &env, { })), nullptr, { }))]->begin())->masked());
    }
}

TEST(ERepository, Manifest)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys, std::placeholders::_1))));
    env.add_repository(1, repo);
    repo->make_manifest(QualifiedPackageName("category/package"));

    EXPECT_EQ(contents("e_repository_TEST_dir/repo11/Manifest_correct"), contents("e_repository_TEST_dir/repo11/category/package/Manifest"));

    EXPECT_THROW(repo->make_manifest(QualifiedPackageName("category/package-b")), MissingDistfileError);

    std::shared_ptr<Map<std::string, std::string> > keys2(std::make_shared<Map<std::string, std::string>>());
    keys2->insert("format", "e");
    keys2->insert("names_cache", "/var/empty");
    keys2->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11a"));
    keys2->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11a/profiles/profile"));
    keys2->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo2(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys2, std::placeholders::_1))));
    env.add_repository(1, repo2);
    repo2->make_manifest(QualifiedPackageName("category/package"));

    EXPECT_EQ(contents("e_repository_TEST_dir/repo11a/Manifest_correct"), contents("e_repository_TEST_dir/repo11a/category/package/Manifest"));

    std::shared_ptr<Map<std::string, std::string> > keys3(std::make_shared<Map<std::string, std::string>>());
    keys3->insert("format", "e");
    keys3->insert("names_cache", "/var/empty");
    keys3->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11b"));
    keys3->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11b/profiles/profile"));
    keys3->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo3(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys3, std::placeholders::_1))));
    env.add_repository(1, repo3);
    repo3->make_manifest(QualifiedPackageName("category/package"));

    EXPECT_EQ(contents("e_repository_TEST_dir/repo11b/Manifest_correct"), contents("e_repository_TEST_dir/repo11b/category/package/Manifest"));

    EXPECT_TRUE(FSStat(FSPath("e_repository_TEST_dir/repo11b/category/package2/Manifest")).exists());
    repo3->make_manifest(QualifiedPackageName("category/package2"));
    EXPECT_TRUE(! FSStat(FSPath("e_repository_TEST_dir/repo11b/category/package2/Manifest")).exists());
}

TEST(ERepository, Fetch)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "exheres");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo12"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo12/profiles/profile"));
    keys->insert("layout", "exheres");
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("profile_eapi", "exheres-0");
    keys->insert("distdir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "distdir"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    FetchAction action(make_named_values<FetchActionOptions>(
                n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                n::exclude_unmirrorable() = false,
                n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
                n::ignore_not_in_manifest() = false,
                n::ignore_unfetched() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::safe_resume() = true,
                n::want_phase() = &want_all_phases
            ));

    {
        const std::shared_ptr<const PackageID> no_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/no-files",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(no_files_id));
        ASSERT_TRUE(bool(no_files_id->short_description_key()));
        EXPECT_EQ("The Short Description", no_files_id->short_description_key()->parse_value());
        no_files_id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> fetched_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(fetched_files_id));
        EXPECT_TRUE((FSPath("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").stat().is_regular_file());
        fetched_files_id->perform_action(action);
        EXPECT_TRUE((FSPath("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").stat().is_regular_file());
    }

    {
        EXPECT_TRUE(! (FSPath("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").stat().is_regular_file());
        const std::shared_ptr<const PackageID> fetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(fetchable_files_id));
        fetchable_files_id->perform_action(action);
        EXPECT_TRUE((FSPath("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").stat().is_regular_file());
    }

    {
        EXPECT_TRUE(! (FSPath("e_repository_TEST_dir") / "distdir" / "arrowed.txt").stat().is_regular_file());
        const std::shared_ptr<const PackageID> arrow_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/arrow-files",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(arrow_files_id));
        arrow_files_id->perform_action(action);
        EXPECT_TRUE((FSPath("e_repository_TEST_dir") / "distdir" / "arrowed.txt").stat().is_regular_file());
    }

    {
        const std::shared_ptr<const PackageID> unfetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/unfetchable-files",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(unfetchable_files_id));
        EXPECT_THROW(unfetchable_files_id->perform_action(action), ActionFailedError);
    }

    {
        const std::shared_ptr<const PackageID> no_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/no-files-restricted",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(no_files_restricted_id));
        no_files_restricted_id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> fetched_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files-restricted",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(fetched_files_restricted_id));
        fetched_files_restricted_id->perform_action(action);
    }

    {
        const std::shared_ptr<const PackageID> fetchable_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                        PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files-restricted",
                                &env, { })), nullptr, { }))]->last());
        ASSERT_TRUE(bool(fetchable_files_restricted_id));
        EXPECT_THROW(fetchable_files_restricted_id->perform_action(action), ActionFailedError);
    }
}

TEST(ERepository, ManifestCheck)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys, std::placeholders::_1))));
    env.add_repository(1, repo);

    FetchAction action(make_named_values<FetchActionOptions>(
                n::errors() = std::make_shared<Sequence<FetchActionFailure>>(),
                n::exclude_unmirrorable() = false,
                n::fetch_parts() = FetchParts() + fp_regulars + fp_extras,
                n::ignore_not_in_manifest() = false,
                n::ignore_unfetched() = false,
                n::make_output_manager() = &make_standard_output_manager,
                n::safe_resume() = true,
                n::want_phase() = &want_all_phases
            ));

    const std::shared_ptr<const PackageID> id(*env[selection::AllVersionsSorted(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("category/package",
                            &env, { })), nullptr, { }))]->last());
    ASSERT_TRUE(bool(id));
    repo->make_manifest(id->name());
    id->perform_action(action);
}

TEST(ERepository, ParseEAPI)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("eapi_when_unspecified", "paludis-1");
    keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo20"));
    keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo20/profiles/profile"));
    keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
    std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                    std::bind(from_keys, keys, std::placeholders::_1))));
    env.add_repository(1, repo);

    const std::string eapis[] = {
        "0", "0", "paludis-1", "0", "1", "1", "1", "1", "1", "1",
        "1", "1", "1", "1", "1", "0", "not-a-real-eapi", "UNKNOWN", "UNKNOWN", "1",
        "UNKNOWN", "0", "1", "UNKNOWN", "UNKNOWN", "1", "UNKNOWN", "UNKNOWN", "UNKNOWN", "1",
        "0", "UNKNOWN", "UNKNOWN", "exheres-0", "exheres-0", "UNKNOWN", ""
    };
    auto ids(env[selection::AllVersionsSorted(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("category/package",
                            &env, { })), nullptr, { }))]);

    int i(0);
    for (auto it(ids->begin()), it_end(ids->end()); it_end != it; ++i, ++it)
        EXPECT_EQ(eapis[i], std::static_pointer_cast<const MetadataValueKey<std::string> >(*(*it)->find_metadata("EAPI"))->parse_value()) << "(i == " << i << ")";
    EXPECT_EQ("", eapis[i]) << "(i == " << i << ")";
}

