/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007, 2008 Ciaran McCreesh
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
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repository_maker.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/set.hh>
#include <paludis/util/kc.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <paludis/user_dep_spec.hh>
#include <paludis/generator.hh>
#include <paludis/filter.hh>
#include <paludis/filtered_generator.hh>
#include <paludis/selection.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <tr1/functional>
#include <set>
#include <fstream>
#include <string>

#include "config.h"

using namespace test;
using namespace paludis;

namespace
{
    std::string from_keys(const std::tr1::shared_ptr<const Map<std::string, std::string> > & m,
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
#if 0
    struct ERepositoryRepoNameTest : TestCase
    {
        ERepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_e_repository_repo_name;

    struct ERepositoryNoRepoNameTest : TestCase
    {
        ERepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo2");
            keys->insert("profiles", "e_repository_TEST_dir/repo2/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo2");
        }
    } test_e_repository_no_repo_name;

    struct ERepositoryEmptyRepoNameTest : TestCase
    {
        ERepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo3");
            keys->insert("profiles", "e_repository_TEST_dir/repo3/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo3");
        }
    } test_e_repository_empty_repo_name;

    struct ERepositoryHasCategoryNamedTest : TestCase
    {
        ERepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
                TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-three")));
                TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-four")));
            }
        }
    } test_e_repository_has_category_named;

    struct ERepositoryCategoryNamesTest : TestCase
    {
        ERepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const CategoryNamePartSet> c(repo->category_names());
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-one")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-two")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-three")));
                TEST_CHECK(c->end() == c->find(CategoryNamePart("cat-four")));
                TEST_CHECK_EQUAL(3, std::distance(c->begin(), c->end()));
            }
        }
    } test_e_repository_category_names;

    struct ERepositoryHasPackageNamedTest : TestCase
    {
        ERepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_e_repository_has_package_named;

    struct ERepositoryHasPackageNamedCachedTest : TestCase
    {
        ERepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            repo->package_names(CategoryNamePart("cat-one"));
            repo->package_names(CategoryNamePart("cat-two"));
            repo->package_names(CategoryNamePart("cat-three"));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(repo->has_package_named(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(! repo->has_package_named(QualifiedPackageName("cat-three/pkg-neither")));
            }
        }
    } test_e_repository_has_package_named_cached;

    struct ERepositoryPackageNamesTest : TestCase
    {
        ERepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> names;

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-one/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-two"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-one")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-both")));
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-two/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-two/pkg-neither")));
                TEST_CHECK_EQUAL(2, std::distance(names->begin(), names->end()));

                names = repo->package_names(CategoryNamePart("cat-three"));
                TEST_CHECK(names->empty());
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-one")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-both")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-two")));
                TEST_CHECK(names->end() == names->find(QualifiedPackageName("cat-three/pkg-neither")));
                TEST_CHECK_EQUAL(0, std::distance(names->begin(), names->end()));
            }
        }
    } test_e_repository_package_names;

    struct ERepositoryBadPackageNamesTest : TestCase
    {
        ERepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo5");
            keys->insert("profiles", "e_repository_TEST_dir/repo5/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            std::tr1::shared_ptr<const QualifiedPackageNameSet> names;

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK_EQUAL(1, std::distance(names->begin(), names->end()));
            }
        }
    } test_e_repository_bad_package_names;

    struct ERepositoryPackageIDTest : TestCase
    {
        ERepositoryPackageIDTest() : TestCase("package_ids") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_versions;

    struct ERepositoryDuffVersionsTest : TestCase
    {
        ERepositoryDuffVersionsTest() : TestCase("duff versions") { }

        void run()
        {
            using namespace std::tr1::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo8");
            keys->insert("profiles", "e_repository_TEST_dir/repo8/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::tr1::bind(std::equal_to<VersionSpec>(), std::tr1::bind(std::tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_duff_versions;

    struct ERepositoryMetadataCachedTest : TestCase
    {
        ERepositoryMetadataCachedTest() : TestCase("metadata cached") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo6");
            keys->insert("profiles", "e_repository_TEST_dir/repo6/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());

                TEST_CHECK(id->short_description_key());
                TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description");
            }
        }
    } test_e_repository_metadata_cached;
#endif

    struct ERepositoryMetadataUncachedTest : TestCase
    {
        ERepositoryMetadataUncachedTest() : TestCase("metadata uncached") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                env.set_paludis_command("/bin/false");
                std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                        new Map<std::string, std::string>);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", "e_repository_TEST_dir/repo7/metadata/cache");
                keys->insert("location", "e_repository_TEST_dir/repo7");
                keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
                std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                            &env, UserPackageDepSpecOptions()))))]->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI")));
                    TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI"))->value(), "0");
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Description");
                    StringifyFormatter ff;
                    erepository::DepSpecPrettyPrinter pd(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id1->build_dependencies_key());
                    id1->build_dependencies_key()->value()->accept(pd);
                    TEST_CHECK_STRINGIFY_EQUAL(pd, "foo/bar");
                    erepository::DepSpecPrettyPrinter pr(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id1->run_dependencies_key());
                    id1->run_dependencies_key()->value()->accept(pr);
                    TEST_CHECK_STRINGIFY_EQUAL(pr, "foo/bar");

                    const std::tr1::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                            &env, UserPackageDepSpecOptions()))))]->begin());

                    TEST_CHECK(id2->end_metadata() != id2->find_metadata("EAPI"));
                    TEST_CHECK(id2->short_description_key());
                    TEST_CHECK_EQUAL(id2->short_description_key()->value(), "dquote \" squote ' backslash \\ dollar $");
                    erepository::DepSpecPrettyPrinter pd2(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id2->build_dependencies_key());
                    id2->build_dependencies_key()->value()->accept(pd2);
                    TEST_CHECK_STRINGIFY_EQUAL(pd2, "foo/bar bar/baz");
                    erepository::DepSpecPrettyPrinter pr2(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id2->run_dependencies_key());
                    id2->run_dependencies_key()->value()->accept(pr2);
                    TEST_CHECK_STRINGIFY_EQUAL(pr2, "foo/bar");
                }
            }
        }
    } test_e_repository_metadata_uncached;

#if 0
    struct ERepositoryMetadataStaleTest : TestCase
    {
        ERepositoryMetadataStaleTest() : TestCase("metadata stale") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                env.set_paludis_command("/bin/false");
                std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                        new Map<std::string, std::string>);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", "e_repository_TEST_dir/repo7/metadata/cache");
                keys->insert("location", "e_repository_TEST_dir/repo7");
                keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
                std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                            std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/stale-pkg-1",
                                            &env, UserPackageDepSpecOptions()))))]->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description");
                }

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/stale-pkg-1",
                                            &env, UserPackageDepSpecOptions()))))]->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description");
                }
            }
        }
    } test_e_repository_metadata_stale;

    struct ERepositoryMetadataUnparsableTest : TestCase
    {
        ERepositoryMetadataUnparsableTest() : TestCase("metadata unparsable") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        unsigned max_run_time() const
        {
            return 3000;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo7");
            keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::tr1::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());

                TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                TEST_CHECK_EQUAL(std::tr1::static_pointer_cast<const erepository::ERepositoryID>(id1)->eapi()->name(), "UNKNOWN");
                TEST_CHECK(! id1->short_description_key());
            }
        }
    } test_e_repository_metadata_unparsable;

    struct ERepositoryQueryUseTest : TestCase
    {
        ERepositoryQueryUseTest() : TestCase("USE query") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo9");
            keys->insert("profiles", "e_repository_TEST_dir/repo9/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::tr1::shared_ptr<const PackageID> p1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> p2(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-two/pkg-two-1",
                                        &env, UserPackageDepSpecOptions()))))]->begin());
                const std::tr1::shared_ptr<const PackageID> p4(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                        &env, UserPackageDepSpecOptions()))))]->begin());

                TEST_CHECK(repo->query_use(UseFlagName("flag1"), *p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag2"), *p1) == use_disabled);
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag2"), *p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), *p2));
                TEST_CHECK(! repo->query_use_mask(UseFlagName("flag3"), *p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), *p4));
                TEST_CHECK(repo->query_use(UseFlagName("flag3"), *p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), *p2) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), *p1) == use_unspecified);
                TEST_CHECK(repo->query_use(UseFlagName("test"), *p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("test2"), *p1) == use_disabled);
                TEST_CHECK(! repo->query_use_mask(UseFlagName("test"), *p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("test2"), *p1));
            }
        }
    } test_e_repository_query_use;

    struct ERepositoryRepositoryMasksTest : TestCase
    {
        ERepositoryRepositoryMasksTest() : TestCase("repository masks") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys18(
                    new Map<std::string, std::string>);
            keys18->insert("format", "ebuild");
            keys18->insert("names_cache", "/var/empty");
            keys18->insert("location", "e_repository_TEST_dir/repo18");
            keys18->insert("profiles", "e_repository_TEST_dir/repo18/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo18(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys18, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo18);

            std::tr1::shared_ptr<Map<std::string, std::string> > keys19(
                    new Map<std::string, std::string>);
            keys19->insert("format", "ebuild");
            keys19->insert("names_cache", "/var/empty");
            keys19->insert("location", "e_repository_TEST_dir/repo19");
            keys19->insert("master_repository", "test-repo-18");
            std::tr1::shared_ptr<ERepository> repo19(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys19, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo19);

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-18",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-18",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-18",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-18",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-19",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-19",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-19",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-19",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
        }
    } test_e_repository_repository_masks;

    struct ERepositoryQueryProfileMasksTest : TestCase
    {
        ERepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo10");
            keys->insert("profiles", "e_repository_TEST_dir/repo10/profiles/profile/subprofile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/masked-0",
                                            &env, UserPackageDepSpecOptions()))))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                            &env, UserPackageDepSpecOptions()))))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/not_masked-0",
                                            &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            }
        }
    } test_e_repository_query_profile_masks;

    struct ERepositoryInvalidateMasksTest : TestCase
    {
        ERepositoryInvalidateMasksTest() : TestCase("invalidate_masks") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo10");
            keys->insert("profiles", "e_repository_TEST_dir/repo10/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location / "profiles/profile/subprofile"));
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location / "profiles/profile"));
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        &env, UserPackageDepSpecOptions()))))]->begin())->masked());
        }
    } test_e_repository_invalidate_masks;

    struct ERepositoryVirtualsTest : TestCase
    {
        ERepositoryVirtualsTest() : TestCase("virtuals") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo15");
            keys->insert("profiles", "e_repository_TEST_dir/repo15/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            bool has_one(false), has_two(false), has_three(false);
            int count(0);

            std::tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> seq(repo->virtual_packages());
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify((*it)[k::virtual_name()]))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*(*it)[k::provided_by_spec()], "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/two");
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*(*it)[k::provided_by_spec()], "cat-two/pkg-two");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK_EQUAL(count, 2);

            repo->set_profile(repo->find_profile(repo->params().location / "profiles/profile/subprofile"));

            has_one = has_two = false;
            count = 0;

            seq = repo->virtual_packages();
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify((*it)[k::virtual_name()]))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*(*it)[k::provided_by_spec()], "cat-two/pkg-two");
                }
                else if ("virtual/two" == stringify((*it)[k::virtual_name()]))
                {
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*(*it)[k::provided_by_spec()], "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL((*it)[k::virtual_name()], "virtual/three");
                    has_three = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*(*it)[k::provided_by_spec()], "cat-three/pkg-three");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK(has_three);
            TEST_CHECK_EQUAL(count, 3);
        }
    } test_e_repository_virtuals;

    struct ERepositoryManifestTest : TestCase
    {
        ERepositoryManifestTest() : TestCase("manifest2") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo11");
            keys->insert("profiles", "e_repository_TEST_dir/repo11/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            repo->make_manifest(QualifiedPackageName("category/package"));

            std::multiset<std::string> made_manifest, reference_manifest;
            std::ifstream made_manifest_stream("e_repository_TEST_dir/repo11/category/package/Manifest"),
                reference_manifest_stream("e_repository_TEST_dir/repo11/Manifest_correct");

            std::string line;

            while ( getline(made_manifest_stream, line) )
                made_manifest.insert(line);
            while ( getline(reference_manifest_stream, line) )
                reference_manifest.insert(line);

            TEST_CHECK(made_manifest == reference_manifest);

            TEST_CHECK_THROWS(repo->make_manifest(QualifiedPackageName("category/package-b")), ERepositoryConfigurationError);
        }
    } test_e_repository_manifest;

    struct ERepositoryFetchTest : TestCase
    {
        ERepositoryFetchTest() : TestCase("fetch") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exheres");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo12");
            keys->insert("profiles", "e_repository_TEST_dir/repo12/profiles/profile");
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            FetchAction action(FetchActionOptions::named_create()
                    (k::fetch_unneeded(), false)
                    (k::safe_resume(), true)
                    );

            {
                TestMessageSuffix suffix("no files", true);
                const std::tr1::shared_ptr<const PackageID> no_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(no_files_id);
                TEST_CHECK(no_files_id->short_description_key());
                TEST_CHECK_EQUAL(no_files_id->short_description_key()->value(), "The Description");
                no_files_id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fetched files", true);
                const std::tr1::shared_ptr<const PackageID> fetched_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(fetched_files_id);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
                fetched_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("fetchable files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
                const std::tr1::shared_ptr<const PackageID> fetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(fetchable_files_id);
                fetchable_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("arrow files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
                const std::tr1::shared_ptr<const PackageID> arrow_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/arrow-files",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(arrow_files_id);
                arrow_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("unfetchable files", true);
                const std::tr1::shared_ptr<const PackageID> unfetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unfetchable-files",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(unfetchable_files_id);
                TEST_CHECK_THROWS(unfetchable_files_id->perform_action(action), FetchActionError);
            }

            {
                const std::tr1::shared_ptr<const PackageID> no_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files-restricted",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(no_files_restricted_id);
                no_files_restricted_id->perform_action(action);
            }

            {
                const std::tr1::shared_ptr<const PackageID> fetched_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files-restricted",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(fetched_files_restricted_id);
                fetched_files_restricted_id->perform_action(action);
            }

            {
                const std::tr1::shared_ptr<const PackageID> fetchable_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files-restricted",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(fetchable_files_restricted_id);
                TEST_CHECK_THROWS(fetchable_files_restricted_id->perform_action(action), FetchActionError);
            }
        }
    } test_e_repository_fetch;

    struct ERepositoryManifestCheckTest : TestCase
    {
        ERepositoryManifestCheckTest() : TestCase("manifest_check") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo11");
            keys->insert("profiles", "e_repository_TEST_dir/repo11/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            FetchAction action(FetchActionOptions::named_create()
                    (k::fetch_unneeded(), false)
                    (k::safe_resume(), true)
                    );

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::AllVersionsSorted(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions()))))]->last());
            TEST_CHECK(id);
            repo->make_manifest(id->name());
            id->perform_action(action);
        }
    } test_e_repository_manifest_check;

    struct ERepositoryInstallEAPI0Test : TestCase
    {
        ERepositoryInstallEAPI0Test() : TestCase("install_eapi_0") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif

            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo13");
            keys->insert("profiles", "e_repository_TEST_dir/repo13/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("installed_virtuals")(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("virtuals")(&env,
                        std::tr1::bind(from_keys, make_shared_ptr(new Map<std::string, std::string>), std::tr1::placeholders::_1)));
#endif

            InstallAction action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );

#ifdef ENABLE_VIRTUALS_REPOSITORY
            {
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=virtual/virtual-pretend-installed-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
            }
#endif

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/econf-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("emake die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("einstall die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/einstall-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("keepdir die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dobin die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fperms die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf source 0", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "0");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/vars-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expand vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/expand-vars-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_0;

    struct ERepositoryInstallEAPI1Test : TestCase
    {
        ERepositoryInstallEAPI1Test() : TestCase("install_eapi_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo13");
            keys->insert("profiles", "e_repository_TEST_dir/repo13/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );

            {
                TestMessageSuffix suffix("econf source 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dosym success 1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-success-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_1;

    struct ERepositoryInstallEAPIKdebuild1Test : TestCase
    {
        ERepositoryInstallEAPIKdebuild1Test() : TestCase("install_eapi_kdebuild_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo13");
            keys->insert("profiles", "e_repository_TEST_dir/repo13/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );

            {
                TestMessageSuffix suffix("econf source kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("banned functions kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-functions-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("banned vars kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/banned-vars-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "UNKNOWN");
            }

            {
                TestMessageSuffix suffix("dosym success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dosym fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/dosym-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }
        }
    } test_e_repository_install_eapi_kdebuild_1;

    struct ERepositoryInfoEAPIKdebuild1Test : TestCase
    {
        ERepositoryInfoEAPIKdebuild1Test() : TestCase("info_eapi_kdebuild_1") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo13");
            keys->insert("profiles", "e_repository_TEST_dir/repo13/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InfoAction action;

            {
                TestMessageSuffix suffix("info success kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-success-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("info fail kdebuild-1", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/info-fail-kdebuild-1",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataValueKey<std::string> >(**id->find_metadata("EAPI"))->value(), "kdebuild-1");
                TEST_CHECK_THROWS(id->perform_action(action), InfoActionError);
            }
        }
    } test_e_repository_info_eapi_kdebuild_1;

    struct ERepositoryInstallExheres0Test : TestCase
    {
        ERepositoryInstallExheres0Test() : TestCase("install_exheres_0") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
#ifdef ENABLE_VIRTUALS_REPOSITORY
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "yes", 1);
#else
            ::setenv("PALUDIS_ENABLE_VIRTUALS_REPOSITORY", "", 1);
#endif
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo14");
            keys->insert("profiles", "e_repository_TEST_dir/repo14/profiles/profile");
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            std::tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

#ifdef ENABLE_VIRTUALS_REPOSITORY
            std::tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("installed_virtuals")(&env,
                        std::tr1::bind(from_keys, iv_keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("virtuals")(&env,
                        std::tr1::bind(from_keys, make_shared_ptr(new Map<std::string, std::string>), std::tr1::placeholders::_1)));
#endif

            InstallAction action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expatch success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/expatch-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expatch die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/expatch-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal expatch fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal expatch die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-expatch-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal unpack fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal unpack die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-unpack-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/econf-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal econf", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal econf die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-econf-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal emake", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal emake die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-emake-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("einstall fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/einstall-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal einstall", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-einstall",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal einstall die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-einstall-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("keepdir success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("keepdir fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/keepdir-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal keepdir", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal keepdir die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-keepdir-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("dobin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/dobin-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal dobin die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-dobin-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("fperms success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fperms-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms success", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-success",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms fail", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-fail",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("nonfatal fperms die", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/nonfatal-fperms-die",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("ever", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/ever-1.3",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK(id->short_description_key());
                TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Description");
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("econf phase", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-phase-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("econf vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-vars-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("expand vars", true);
                const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/expand-vars-0",
                                        &env, UserPackageDepSpecOptions()))))]->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }
        }
    } test_e_repository_install_exheres_0;

    struct ERepositoryDependenciesRewriterTest : TestCase
    {
        ERepositoryDependenciesRewriterTest() : TestCase("dependencies_rewriter") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo17");
            keys->insert("profiles", "e_repository_TEST_dir/repo17/profiles/profile");
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    &env, UserPackageDepSpecOptions()))))]->last());

            StringifyFormatter ff;

            erepository::DepSpecPrettyPrinter pd(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->build_dependencies_key());
            id->build_dependencies_key()->value()->accept(pd);
            TEST_CHECK_STRINGIFY_EQUAL(pd, "cat/pkg1 build: cat/pkg2 build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pr(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->run_dependencies_key());
            id->run_dependencies_key()->value()->accept(pr);
            TEST_CHECK_STRINGIFY_EQUAL(pr, "cat/pkg1 build: build,run: cat/pkg3 suggested: cat/pkg4 post:");

            erepository::DepSpecPrettyPrinter pp(0, std::tr1::shared_ptr<const PackageID>(), ff, 0, false);
            TEST_CHECK(id->post_dependencies_key());
            id->post_dependencies_key()->value()->accept(pp);
            TEST_CHECK_STRINGIFY_EQUAL(pp, "build: build,run: suggested: post: cat/pkg5");
        }
    } test_e_repository_dependencies_rewriter;

    struct ERepositorySymlinkRewritingTest : TestCase
    {
        ERepositorySymlinkRewritingTest() : TestCase("symlink_rewriting") { }

        unsigned max_run_time() const
        {
            return 3000;
        }

        bool repeatable() const
        {
            return false;
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo20");
            keys->insert("profiles", "e_repository_TEST_dir/repo20/profiles/profile");
            keys->insert("layout", "traditional");
            keys->insert("eapi_when_unknown", "0");
            keys->insert("eapi_when_unspecified", "0");
            keys->insert("profile_eapi", "0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "symlinked_build"));
            keys->insert("root", stringify(FSEntry("e_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            keys.reset(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/vdb");
            keys->insert("builddir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "build"));
            keys->insert("root", stringify(FSEntry("e_repository_TEST_dir/root").realpath()));
            std::tr1::shared_ptr<Repository> installed_repo(VDBRepository::make_vdb_repository(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1)));
            env.package_database()->add_repository(1, installed_repo);

            InstallAction action(InstallActionOptions::named_create()
                    (k::debug_build(), iado_none)
                    (k::checks(), iaco_default)
                    (k::destination(), installed_repo)
                    );

            const std::tr1::shared_ptr<const PackageID> id(*env[selection::RequireExactlyOne(generator::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("cat/pkg",
                                    &env, UserPackageDepSpecOptions()))))]->last());
            TEST_CHECK(id);

            id->perform_action(action);
            TEST_CHECK_EQUAL(FSEntry("e_repository_TEST_dir/root/bar").readlink(), "/foo");
        }
    } test_e_repository_symlink_rewriting;
#endif
}

