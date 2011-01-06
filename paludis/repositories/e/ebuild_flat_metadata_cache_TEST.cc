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
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/safe_ifstream.hh>
#include <paludis/util/timestamp.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <iterator>

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
    struct MetadataFlatListCachedTest : TestCase
    {
        MetadataFlatListCachedTest() : TestCase("metadata flat_list cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_list");
        }
    } test_metadata_flat_list_cached;

    struct MetadataFlatListStaleTest : TestCase
    {
        MetadataFlatListStaleTest() : TestCase("metadata flat_list stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_list-stale");
        }
    } test_metadata_flat_list_stale;

    struct MetadataFlatListGuessedEAPITest : TestCase
    {
        MetadataFlatListGuessedEAPITest() : TestCase("metadata flat_list guessed EAPI") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-guessed-eapi-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_list-guessed-eapi");
        }
    } test_metadata_flat_list_guessed_eapi;

    struct MetadataFlatListEclassCachedTest : TestCase
    {
        MetadataFlatListEclassCachedTest() : TestCase("metadata flat_list eclass cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_list-eclass");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "foo");
        }
    } test_metadata_flat_list_eclass_cached;

    struct MetadataFlatListEclassStaleTest : TestCase
    {
        MetadataFlatListEclassStaleTest() : TestCase("metadata flat_list eclass stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_list-eclass-stale");
        }
    } test_metadata_flat_list_eclass_stale;

    struct MetadataFlatListEclassWrongTest : TestCase
    {
        MetadataFlatListEclassWrongTest() : TestCase("metadata flat_list eclass wrong") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-wrong-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_list-eclass-wrong");
        }
    } test_metadata_flat_list_eclass_wrong;

    struct MetadataFlatListEclassGoneTest : TestCase
    {
        MetadataFlatListEclassGoneTest() : TestCase("metadata flat_list eclass gone") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-eclass-gone-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_list-eclass-gone");
        }
    } test_metadata_flat_list_eclass_gone;

    struct MetadataFlatListDetectionTest : TestCase
    {
        MetadataFlatListDetectionTest() : TestCase("metadata flat_list detection") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_list-detection-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_list-detection");
        }
    } test_metadata_flat_list_detection;

    struct MetadataFlatHashCachedTest : TestCase
    {
        MetadataFlatHashCachedTest() : TestCase("metadata flat_hash cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash");
        }
    } test_metadata_flat_hash_cached;

    struct MetadataFlatHashGuessedEAPITest : TestCase
    {
        MetadataFlatHashGuessedEAPITest() : TestCase("metadata flat_hash guessed EAPI") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-guessed-eapi-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-guessed-eapi");
        }
    } test_metadata_flat_hash_guessed_eapi;

    struct MetadataFlatHashGuessedEAPIExtensionTest : TestCase
    {
        MetadataFlatHashGuessedEAPIExtensionTest() : TestCase("metadata flat_hash guessed EAPI by extension") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-guessed-eapi-extension-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-guessed-eapi-extension");
        }
    } test_metadata_flat_hash_guessed_eapi_extension;

    struct MetadataFlatHashCachedNoGuessedEAPITest : TestCase
    {
        MetadataFlatHashCachedNoGuessedEAPITest() : TestCase("metadata flat_hash no guessed EAPI") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-guessed-eapi-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-no-guessed-eapi");
        }
    } test_metadata_flat_hash_cached_no_guessed_eapi;

    struct MetadataFlatHashEmptyValueTest : TestCase
    {
        MetadataFlatHashEmptyValueTest() : TestCase("metadata flat_hash empty value") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-empty-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "");
            TEST_CHECK_STRINGIFY_EQUAL(id->slot_key()->value(), "the-slot");
        }
    } test_metadata_flat_hash_empty_value;

    struct MetadataFlatHashStaleTest : TestCase
    {
        MetadataFlatHashStaleTest() : TestCase("metadata flat_hash stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-stale");
        }
    } test_metadata_flat_hash_stale;

    struct MetadataFlatHashNoMtimeTest : TestCase
    {
        MetadataFlatHashNoMtimeTest() : TestCase("metadata flat_hash no mtime") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-mtime-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "the-description-flat_hash-no-mtime");
        }
    } test_metadata_flat_hash_no_mtime;

    struct MetadataFlatHashNoMtimeStaleTest : TestCase
    {
        MetadataFlatHashNoMtimeStaleTest() : TestCase("metadata flat_hash no mtime stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-mtime-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-no-mtime-stale");
        }
    } test_metadata_flat_hash_no_mtime_stale;

    struct MetadataFlatHashBadMtimeTest : TestCase
    {
        MetadataFlatHashBadMtimeTest() : TestCase("metadata flat_hash bad mtime") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-bad-mtime-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-bad-mtime");
        }
    } test_metadata_flat_hash_bad_mtime;

    struct MetadataFlatHashNoEAPITest : TestCase
    {
        MetadataFlatHashNoEAPITest() : TestCase("metadata flat_hash no EAPI") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-no-eapi-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-no-eapi");
        }
    } test_metadata_flat_hash_no_eapi;

    struct MetadataFlatHashDuplicateKeyTest : TestCase
    {
        MetadataFlatHashDuplicateKeyTest() : TestCase("metadata flat_hash duplicate key") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-duplicate-key-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id1->short_description_key()));
            TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description flat_hash-duplicate-key");
        }
    } test_metadata_flat_hash_duplicate_key;

    struct MetadataFlatHashEclassTest : TestCase
    {
        MetadataFlatHashEclassTest() : TestCase("metadata flat_hash eclass") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-eclass");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "foo");
        }
    } test_metadata_flat_hash_eclass;

    struct MetadataFlatHashEclassStaleTest : TestCase
    {
        MetadataFlatHashEclassStaleTest() : TestCase("metadata flat_hash eclass stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclass-stale");
        }
    } test_metadata_flat_hash_eclass_stale;

    struct MetadataFlatHashEclassWrongTest : TestCase
    {
        MetadataFlatHashEclassWrongTest() : TestCase("metadata flat_hash eclass wrong") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-wrong-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclass-wrong");
        }
    } test_metadata_flat_hash_eclass_wrong;

    struct MetadataFlatHashEclassGoneTest : TestCase
    {
        MetadataFlatHashEclassGoneTest() : TestCase("metadata flat_hash eclass gone") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclass-gone-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclass-gone");
        }
    } test_metadata_flat_hash_eclass_gone;

    struct MetadataFlatHashFullEclassTest : TestCase
    {
        MetadataFlatHashFullEclassTest() : TestCase("metadata flat_hash full eclass") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-full-eclass");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "foo");
        }
    } test_metadata_flat_hash_full_eclass;

    struct MetadataFlatHashFullEclassNonstandardTest : TestCase
    {
        MetadataFlatHashFullEclassNonstandardTest() : TestCase("metadata flat_hash full eclass nonstandard") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-nonstandard-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-full-eclass-nonstandard");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "bar foo");
        }
    } test_metadata_flat_hash_full_eclass_nonstandard;

    struct MetadataFlatHashFullEclassStaleTest : TestCase
    {
        MetadataFlatHashFullEclassStaleTest() : TestCase("metadata flat_hash full eclass stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-full-eclass-stale");
        }
    } test_metadata_flat_hash_full_eclass_stale;

    struct MetadataFlatHashFullEclassWrongTest : TestCase
    {
        MetadataFlatHashFullEclassWrongTest() : TestCase("metadata flat_hash full eclass wrong") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-wrong-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-full-eclass-wrong");
        }
    } test_metadata_flat_hash_full_eclass_wrong;

    struct MetadataFlatHashFullEclassGoneTest : TestCase
    {
        MetadataFlatHashFullEclassGoneTest() : TestCase("metadata flat_hash full eclass gone") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-full-eclass-gone-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-full-eclass-gone");
        }
    } test_metadata_flat_hash_full_eclass_gone;

    struct MetadataFlatHashEclassesTruncatedTest : TestCase
    {
        MetadataFlatHashEclassesTruncatedTest() : TestCase("metadata flat_hash eclasses truncated") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-truncated-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclasses-truncated");

            std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-truncated-2",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id2->short_description_key()));
            TEST_CHECK_EQUAL(id2->short_description_key()->value(), "The Generated Description flat_hash-eclasses-truncated-2");
        }
    } test_metadata_flat_hash_eclasses_truncated;

    struct MetadataFlatHashEclassesBadMtimeTest : TestCase
    {
        MetadataFlatHashEclassesBadMtimeTest() : TestCase("metadata flat_hash eclasses bad mtime") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-bad-mtime-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclasses-bad-mtime");
        }
    } test_metadata_flat_hash_eclasses_bad_mtime;

    struct MetadataFlatHashEclassesSpacesTest : TestCase
    {
        MetadataFlatHashEclassesSpacesTest() : TestCase("metadata flat_hash eclasses spaces") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo"));
            keys->insert("profiles", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir/repo/profiles/profile"));
            keys->insert("eclassdirs", "ebuild_flat_metadata_cache_TEST_dir/repo/eclass ebuild_flat_metadata_cache_TEST_dir/extra_eclasses");
            keys->insert("builddir", stringify(FSPath::cwd() / "ebuild_flat_metadata_cache_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-eclasses-spaces-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-eclasses-spaces");
        }
    } test_metadata_flat_hash_eclasses_spaces;

    struct MetadataFlatHashExlibTest : TestCase
    {
        MetadataFlatHashExlibTest() : TestCase("metadata flat_hash exlib") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-exlib");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "foo");
        }
    } test_metadata_flat_hash_exlib;

    struct MetadataFlatHashExlibPerCategoryTest : TestCase
    {
        MetadataFlatHashExlibPerCategoryTest() : TestCase("metadata flat_hash exlib per-category") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-percat-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description-flat_hash-exlib-percat");
            TEST_CHECK_EQUAL(join(
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->begin(),
                simple_visitor_cast<const MetadataCollectionKey<Set<std::string> > >(**id->find_metadata("INHERITED"))->value()->end(), " "), "bar foo");
        }
    } test_metadata_flat_hash_exlib_per_category;

    struct MetadataFlatHashExlibStaleTest : TestCase
    {
        MetadataFlatHashExlibStaleTest() : TestCase("metadata flat_hash exlib stale") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-stale-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlib-stale");
        }
    } test_metadata_flat_hash_exlib_stale;

    struct MetadataFlatHashExlibWrongTest : TestCase
    {
        MetadataFlatHashExlibWrongTest() : TestCase("metadata flat_hash exlib wrong") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-wrong-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlib-wrong");
        }
    } test_metadata_flat_hash_exlib_wrong;

    struct MetadataFlatHashExlibGoneTest : TestCase
    {
        MetadataFlatHashExlibGoneTest() : TestCase("metadata flat_hash exlib gone") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlib-gone-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlib-gone");
        }
    } test_metadata_flat_hash_exlib_gone;

    struct MetadataFlatHashExlibsTruncatedTest : TestCase
    {
        MetadataFlatHashExlibsTruncatedTest() : TestCase("metadata flat_hash exlibs truncated") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-truncated-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlibs-truncated");

            std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-truncated-2",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id2->short_description_key()));
            TEST_CHECK_EQUAL(id2->short_description_key()->value(), "The Generated Description flat_hash-exlibs-truncated-2");
        }
    } test_metadata_flat_hash_exlibs_truncated;

    struct MetadataFlatHashExlibsBadMtimeTest : TestCase
    {
        MetadataFlatHashExlibsBadMtimeTest() : TestCase("metadata flat_hash exlibs bad mtime") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-bad-mtime-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlibs-bad-mtime");
        }
    } test_metadata_flat_hash_exlibs_bad_mtime;

    struct MetadataFlatHashExlibsSpacesTest : TestCase
    {
        MetadataFlatHashExlibsSpacesTest() : TestCase("metadata flat_hash exlibs spaces") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/flat_hash-exlibs-spaces-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Generated Description flat_hash-exlibs-spaces");
        }
    } test_metadata_flat_hash_exlibs_spaces;

    struct MetadataWriteTest : TestCase
    {
        MetadataWriteTest() : TestCase("metadata write") { }

        std::string contents(const std::string & filename)
        {
            SafeIFStream s(FSPath(filename).realpath());
            return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/write-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-1"));
            TEST_CHECK_EQUAL(FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-1").stat().mtim().seconds(), 60);
        }
    } test_metadata_write;

    struct MetadataWriteEAPI1Test : TestCase
    {
        MetadataWriteEAPI1Test() : TestCase("metadata write EAPI 1") { }

        std::string contents(const std::string & filename)
        {
            SafeIFStream s(FSPath(filename).realpath());
            return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/write-eapi1-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eapi1-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-eapi1-1"));
            TEST_CHECK_EQUAL(FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eapi1-1").stat().mtim().seconds(), 60);
        }
    } test_metadata_write_eapi1;

    struct MetadataWriteEclassesTest : TestCase
    {
        MetadataWriteEclassesTest() : TestCase("metadata write eclasses") { }

        std::string contents(const std::string & filename)
        {
            SafeIFStream s(FSPath(filename).realpath());
            return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/write-eclasses-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eclasses-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-eclasses-1"));
            TEST_CHECK_EQUAL(FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-eclasses-1").stat().mtim().seconds(), 60);
        }
    } test_metadata_write_eclasses;

    struct MetadataWriteExlibsTest : TestCase
    {
        MetadataWriteExlibsTest() : TestCase("metadata write exlibs") { }

        std::string contents(const std::string & filename)
        {
            SafeIFStream s(FSPath(filename).realpath());
            return std::string((std::istreambuf_iterator<char>(s)), std::istreambuf_iterator<char>());
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
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
            env.package_database()->add_repository(1, repo);

            std::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("=cat/write-exlibs-1",
                                    &env, { })), make_null_shared_ptr(), { }))]->begin());

            TEST_CHECK(bool(id->short_description_key()));
            TEST_CHECK_EQUAL(contents("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-exlibs-1"), contents("ebuild_flat_metadata_cache_TEST_dir/cache/expected/cat/write-exlibs-1"));
            TEST_CHECK_EQUAL(FSPath("ebuild_flat_metadata_cache_TEST_dir/cache/test-repo/cat/write-exlibs-1").stat().mtim().seconds(), 60);
        }
    } test_metadata_write_exlibs;
}

