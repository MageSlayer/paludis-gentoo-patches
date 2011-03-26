/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008, 2009, 2010, 2011 Ciaran McCreesh
 * Copyright (c) 2008 David Leverton
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

#include <paludis/environments/test/test_environment.hh>

#include <paludis/filtered_generator.hh>
#include <paludis/generator.hh>
#include <paludis/metadata_key.hh>
#include <paludis/package_id.hh>
#include <paludis/selection.hh>
#include <paludis/user_dep_spec.hh>

#include <paludis/util/map.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/set.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/util/join.hh>
#include <paludis/util/stringify.hh>

#include <iterator>

#include <gtest/gtest.h>

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

    std::string contents(const std::string & filename)
    {
        SafeIFStream s(FSPath(filename).realpath());
        return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
    }
}

TEST(EbuildFlatMetadataCache, FlatList)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_list", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatListStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_list-stale", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, GuessedEAPI)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    keys->insert("eapi_when_unknown", "1");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-guessed-eapi-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_list-guessed-eapi", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, EclassCached)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_list-eclass", id->short_description_key()->value());
    ASSERT_EQ("foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, EclassStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_list-eclass-stale", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, EclassWrong)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-wrong-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_list-eclass-wrong", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, EclassGone)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-gone-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_list-eclass-gone", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatListDetection)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-detection-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_list-detection", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatHash)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatHashGuessedEAPI)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    keys->insert("eapi_when_unknown", "1");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-guessed-eapi-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-guessed-eapi", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatHashGuessedExtension)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-guessed-eapi-extension-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-guessed-eapi-extension", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, FlatHashNoGuessedEAPI)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-guessed-eapi-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-no-guessed-eapi", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, EmptyValue)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-empty-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("", id->short_description_key()->value());
    EXPECT_EQ("the-slot", stringify(id->slot_key()->value()));
}

TEST(EbuildFlatMetadataCache, HashStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-stale", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashNoMTime)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-mtime-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-no-mtime", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashNoMTimeStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-mtime-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-no-mtime-stale", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashBadMtime)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-bad-mtime-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-bad-mtime", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashNoEAPI)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-eapi-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-no-eapi", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashDuplicateKey)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-duplicate-key-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id1->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-duplicate-key", id1->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclass)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-eclass", id->short_description_key()->value());
    ASSERT_EQ("foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, HashEclassStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclass-stale", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclassWrong)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-wrong-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclass-wrong", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclassGone)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-gone-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclass-gone", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashFullEclass)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-full-eclass", id->short_description_key()->value());
    ASSERT_EQ("foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, HashFullEclassNonStandard)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-nonstandard-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-full-eclass-nonstandard", id->short_description_key()->value());
    ASSERT_EQ("bar foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, HashFullEclassStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-full-eclass-stale", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashFullEclassWrong)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-wrong-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-full-eclass-wrong", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashFullEclassGone)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-gone-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-full-eclass-gone", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclassTruncated)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-truncated-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclasses-truncated", id->short_description_key()->value());

    std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-truncated-2",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id2->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclasses-truncated-2", id2->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclassBadMtime)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-bad-mtime-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclasses-bad-mtime", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashEclassSpaces)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-spaces-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-eclasses-spaces", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlib)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-exlib", id->short_description_key()->value());
    ASSERT_EQ("foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, HashExlibPerCategory)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-percat-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("the-description-flat_hash-exlib-percat", id->short_description_key()->value());
    ASSERT_EQ("bar foo", join(
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
        visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "));
}

TEST(EbuildFlatMetadataCache, HashExlibStale)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-stale-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlib-stale", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlibWrong)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-wrong-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlib-wrong", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlibGone)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-gone-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlib-gone", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlibTruncated)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-truncated-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlibs-truncated", id->short_description_key()->value());

    std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-truncated-2",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id2->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlibs-truncated-2", id2->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlibBadMtime)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-bad-mtime-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlibs-bad-mtime", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, HashExlibSpaces)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-spaces-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ("The Generated Description flat_hash-exlibs-spaces", id->short_description_key()->value());
}

TEST(EbuildFlatMetadataCache, Write)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("write_cache", "ebuild_flat_metadata_cache_TEST_dir/cache");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/write-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ(contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-1"));
    EXPECT_EQ(60, FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-1").stat().mtim().seconds());
}

TEST(EbuildFlatMetadataCache, Write1)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
    keys->insert("write_cache", "ebuild_flat_metadata_cache_TEST_dir/cache");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/write-eapi1-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ(contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-eapi1-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eapi1-1"));
    EXPECT_EQ(60, FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eapi1-1").stat().mtim().seconds());
}

TEST(EbuildFlatMetadataCache, WriteEclasses)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eclassdirs",
            stringify((FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/eclass").realpath())
            + " " + stringify((FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/extra_eclasses").realpath()));
    keys->insert("write_cache", "ebuild_flat_metadata_cache_TEST_dir/cache");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/write-eclasses-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ(contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-eclasses-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eclasses-1"));
    EXPECT_EQ(60, FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eclasses-1").stat().mtim().seconds());
}

TEST(EbuildFlatMetadataCache, WriteExlibs)
{
    TestEnvironment env;
    std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
    keys->insert("format", "e");
    keys->insert("names_cache", "/var/empty");
    keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
    keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
    keys->insert("eapi_when_unknown", "exheres-0");
    keys->insert("eapi_when_unspecified", "exheres-0");
    keys->insert("write_cache", "ebuild_flat_metadata_cache_TEST_dir/cache");
    keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
    std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                std::bind(from_keys, keys, std::placeholders::_1)));
    env.add_repository(1, repo);

    std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                    PackageDepSpec(parse_user_package_dep_spec("=cat/write-exlibs-1",
                            &env, { })), make_null_shared_ptr(), { }))]->begin());

    ASSERT_TRUE(bool(id->short_description_key()));
    EXPECT_EQ(contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-exlibs-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-exlibs-1"));
    EXPECT_EQ(60, FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-exlibs-1").stat().mtim().seconds());
}

