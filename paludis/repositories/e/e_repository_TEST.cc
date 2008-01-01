/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh
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
#include <paludis/repositories/e/e_repository_id.hh>
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/repositories/e/eapi.hh>
#include <paludis/repositories/e/dep_spec_pretty_printer.hh>
#include <paludis/repositories/fake/fake_installed_repository.hh>
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/repository_maker.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/visitor_cast.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/map.hh>
#include <paludis/util/set.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/query.hh>
#include <paludis/action.hh>
#include <paludis/stringify_formatter.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

#include <set>
#include <fstream>
#include <string>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for ERepository.
 *
 */

namespace test_cases
{
    /**
     * \test Test ERepository repository names.
     *
     */
    struct ERepositoryRepoNameTest : TestCase
    {
        ERepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_e_repository_repo_name;

    /**
     * \test Test ERepository repository with no names.
     *
     */
    struct ERepositoryNoRepoNameTest : TestCase
    {
        ERepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo2");
            keys->insert("profiles", "e_repository_TEST_dir/repo2/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo2");
        }
    } test_e_repository_no_repo_name;

    /**
     * \test Test ERepository repository empty names.
     *
     */
    struct ERepositoryEmptyRepoNameTest : TestCase
    {
        ERepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo3");
            keys->insert("profiles", "e_repository_TEST_dir/repo3/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo3");
        }
    } test_e_repository_empty_repo_name;

    /**
     * \test Test ERepository repository has_category_named.
     *
     */
    struct ERepositoryHasCategoryNamedTest : TestCase
    {
        ERepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

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

    /**
     * \test Test ERepository category_names.
     *
     */
    struct ERepositoryCategoryNamesTest : TestCase
    {
        ERepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo1");
            keys->insert("profiles", "e_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const CategoryNamePartSet> c(repo->category_names());
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-one")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-two")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-three")));
                TEST_CHECK(c->end() == c->find(CategoryNamePart("cat-four")));
                TEST_CHECK_EQUAL(3, std::distance(c->begin(), c->end()));
            }
        }
    } test_e_repository_category_names;

    /**
     * \test Test ERepository has_package_named.
     *
     */
    struct ERepositoryHasPackageNamedTest : TestCase
    {
        ERepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

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

    /**
     * \test Test ERepository has_package_named cached.
     *
     */
    struct ERepositoryHasPackageNamedCachedTest : TestCase
    {
        ERepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

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

    /**
     * \test Test ERepository package_names.
     *
     */
    struct ERepositoryPackageNamesTest : TestCase
    {
        ERepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const QualifiedPackageNameSet> names;

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

    /**
     * \test Test ERepository bad package names.
     *
     */
    struct ERepositoryBadPackageNamesTest : TestCase
    {
        ERepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo5");
            keys->insert("profiles", "e_repository_TEST_dir/repo5/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const QualifiedPackageNameSet> names;

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
            using namespace tr1::placeholders;

            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo4");
            keys->insert("profiles", "e_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_versions;

    /**
     * \test Test ERepository duff versions.
     *
     */
    struct ERepositoryDuffVersionsTest : TestCase
    {
        ERepositoryDuffVersionsTest() : TestCase("duff versions") { }

        void run()
        {
            using namespace tr1::placeholders;

            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo8");
            keys->insert("profiles", "e_repository_TEST_dir/repo8/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("1"))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1"))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), VersionSpec("2"))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_duff_versions;

    /**
     * \test Test ERepository cached metadata.
     *
     */
    struct ERepositoryMetadataCachedTest : TestCase
    {
        ERepositoryMetadataCachedTest() : TestCase("metadata cached") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo6");
            keys->insert("profiles", "e_repository_TEST_dir/repo6/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                TEST_CHECK(id->short_description_key());
                TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description");
            }
        }
    } test_e_repository_metadata_cached;

    /**
     * \test Test ERepository uncached metadata.
     *
     */
    struct ERepositoryMetadataUncachedTest : TestCase
    {
        ERepositoryMetadataUncachedTest() : TestCase("metadata uncached") { }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                tr1::shared_ptr<Map<std::string, std::string> > keys(
                        new Map<std::string, std::string>);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", "e_repository_TEST_dir/repo7/metadata/cache");
                keys->insert("location", "e_repository_TEST_dir/repo7");
                keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
                tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                            UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Description");
                    StringifyFormatter ff;
                    erepository::DepSpecPrettyPrinter pd(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id1->build_dependencies_key());
                    id1->build_dependencies_key()->value()->accept(pd);
                    TEST_CHECK_STRINGIFY_EQUAL(pd, "foo/bar");
                    erepository::DepSpecPrettyPrinter pr(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id1->run_dependencies_key());
                    id1->run_dependencies_key()->value()->accept(pr);
                    TEST_CHECK_STRINGIFY_EQUAL(pr, "foo/bar");

                    tr1::shared_ptr<const PackageID> id2(*env.package_database()->query(query::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                            UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                    TEST_CHECK(id2->end_metadata() != id2->find_metadata("EAPI"));
                    TEST_CHECK(id2->short_description_key());
                    TEST_CHECK_EQUAL(id2->short_description_key()->value(), "dquote \" squote ' backslash \\ dollar $");
                    erepository::DepSpecPrettyPrinter pd2(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id2->build_dependencies_key());
                    id2->build_dependencies_key()->value()->accept(pd2);
                    TEST_CHECK_STRINGIFY_EQUAL(pd2, "foo/bar bar/baz");
                    erepository::DepSpecPrettyPrinter pr2(0, tr1::shared_ptr<const PackageID>(), ff, 0, false);
                    TEST_CHECK(id2->run_dependencies_key());
                    id2->run_dependencies_key()->value()->accept(pr2);
                    TEST_CHECK_STRINGIFY_EQUAL(pr2, "foo/bar");
                }
            }
        }
    } test_e_repository_metadata_uncached;

    struct ERepositoryMetadataStaleTest : TestCase
    {
        ERepositoryMetadataStaleTest() : TestCase("metadata stale") { }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                tr1::shared_ptr<Map<std::string, std::string> > keys(
                        new Map<std::string, std::string>);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", "e_repository_TEST_dir/repo7/metadata/cache");
                keys->insert("location", "e_repository_TEST_dir/repo7");
                keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
                tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/stale-pkg-1",
                                            UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description");
                }

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/stale-pkg-2",
                                            UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Generated Description");
                }
            }
        }
    } test_e_repository_metadata_stale;

    /**
     * \test Test ERepository unparsable metadata.
     *
     */
    struct ERepositoryMetadataUnparsableTest : TestCase
    {
        ERepositoryMetadataUnparsableTest() : TestCase("metadata unparsable") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo7");
            keys->insert("profiles", "e_repository_TEST_dir/repo7/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository( &env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

                TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                TEST_CHECK_EQUAL(tr1::static_pointer_cast<const erepository::ERepositoryID>(id1)->eapi()->name, "UNKNOWN");
                TEST_CHECK(! id1->short_description_key());
            }
        }
    } test_e_repository_metadata_unparsable;

    /**
     * \test Test ERepository query_use and query_use_mask functions.
     *
     */
    struct ERepositoryQueryUseTest : TestCase
    {
        ERepositoryQueryUseTest() : TestCase("USE query") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo9");
            keys->insert("profiles", "e_repository_TEST_dir/repo9/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageID> p1(*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1", 
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());
                tr1::shared_ptr<const PackageID> p2(*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-two/pkg-two-1", 
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());
                tr1::shared_ptr<const PackageID> p4(*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2", 
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->begin());

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

    /**
     * \test Test ERepository query_profile_masks functions.
     *
     */
    struct ERepositoryQueryProfileMasksTest : TestCase
    {
        ERepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo10");
            keys->insert("profiles", "e_repository_TEST_dir/repo10/profiles/profile/subprofile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK((*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/masked-0",
                                            UserPackageDepSpecOptions()))),
                                qo_require_exactly_one)->begin())->masked());
                TEST_CHECK(! (*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                            UserPackageDepSpecOptions()))),
                                qo_require_exactly_one)->begin())->masked());
                TEST_CHECK(! (*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/not_masked-0",
                                            UserPackageDepSpecOptions()))),
                                qo_require_exactly_one)->begin())->masked());
            }
        }
    } test_e_repository_query_profile_masks;

    /**
     * \test Test ERepository invalidate_masks functions.
     *
     */
    struct ERepositoryInvalidateMasksTest : TestCase
    {
        ERepositoryInvalidateMasksTest() : TestCase("invalidate_masks") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo10");
            keys->insert("profiles", "e_repository_TEST_dir/repo10/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            TEST_CHECK((*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        UserPackageDepSpecOptions()))),
                            qo_require_exactly_one)->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location / "profiles/profile/subprofile"));
            TEST_CHECK(! (*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        UserPackageDepSpecOptions()))),
                            qo_require_exactly_one)->begin())->masked());
            repo->set_profile(repo->find_profile(repo->params().location / "profiles/profile"));
            TEST_CHECK((*env.package_database()->query(query::Matches(PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                        UserPackageDepSpecOptions()))),
                            qo_require_exactly_one)->begin())->masked());
        }
    } test_e_repository_invalidate_masks;

    /**
     * \test Test ERepository virtuals.
     *
     */
    struct ERepositoryVirtualsTest : TestCase
    {
        ERepositoryVirtualsTest() : TestCase("virtuals") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo15");
            keys->insert("profiles", "e_repository_TEST_dir/repo15/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            bool has_one(false), has_two(false), has_three(false);
            int count(0);

            tr1::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> seq(repo->virtual_packages());
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify(it->virtual_name))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec, "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name, "virtual/two");
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec, "cat-two/pkg-two");
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
                if ("virtual/one" == stringify(it->virtual_name))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec, "cat-two/pkg-two");
                }
                else if ("virtual/two" == stringify(it->virtual_name))
                {
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec, "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name, "virtual/three");
                    has_three = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec, "cat-three/pkg-three");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK(has_three);
            TEST_CHECK_EQUAL(count, 3);
        }
    } test_e_repository_virtuals;

    /**
     * \test Test ERepository Manifest2 generation.
     *
     */
    struct ERepositoryManifestTest : TestCase
    {
        ERepositoryManifestTest() : TestCase("manifest2") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo11");
            keys->insert("profiles", "e_repository_TEST_dir/repo11/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
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
        }
    } test_e_repository_manifest;

    struct ERepositoryFetchTest : TestCase
    {
        ERepositoryFetchTest() : TestCase("fetch") { }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "exheres");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo12");
            keys->insert("profiles", "e_repository_TEST_dir/repo12/profiles/profile");
            keys->insert("layout", "exheres");
            keys->insert("eapi_when_unknown", "exheres-0");
            keys->insert("eapi_when_unspecified", "exheres-0");
            keys->insert("profile_eapi", "exheres-0");
            keys->insert("distdir", stringify(FSEntry::cwd() / "e_repository_TEST_dir" / "distdir"));
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            FetchAction action(FetchActionOptions::create()
                    .fetch_unneeded(false)
                    .safe_resume(true)
                    );

            {
                TestMessageSuffix suffix("no files", true);
                const tr1::shared_ptr<const PackageID> no_files_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(no_files_id);
                TEST_CHECK(no_files_id->short_description_key());
                TEST_CHECK_EQUAL(no_files_id->short_description_key()->value(), "The Description");
                no_files_id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fetched files", true);
                const tr1::shared_ptr<const PackageID> fetched_files_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(fetched_files_id);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
                fetched_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("fetchable files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
                const tr1::shared_ptr<const PackageID> fetchable_files_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(fetchable_files_id);
                fetchable_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("arrow files", true);
                TEST_CHECK(! (FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
                const tr1::shared_ptr<const PackageID> arrow_files_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/arrow-files",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(arrow_files_id);
                arrow_files_id->perform_action(action);
                TEST_CHECK((FSEntry("e_repository_TEST_dir") / "distdir" / "arrowed.txt").is_regular_file());
            }

            {
                TestMessageSuffix suffix("unfetchable files", true);
                const tr1::shared_ptr<const PackageID> unfetchable_files_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unfetchable-files",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(unfetchable_files_id);
                TEST_CHECK_THROWS(unfetchable_files_id->perform_action(action), FetchActionError);
            }

            {
                const tr1::shared_ptr<const PackageID> no_files_restricted_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files-restricted",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(no_files_restricted_id);
                no_files_restricted_id->perform_action(action);
            }

            {
                const tr1::shared_ptr<const PackageID> fetched_files_restricted_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files-restricted",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(fetched_files_restricted_id);
                fetched_files_restricted_id->perform_action(action);
            }

            {
                const tr1::shared_ptr<const PackageID> fetchable_files_restricted_id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files-restricted",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(fetchable_files_restricted_id);
                TEST_CHECK_THROWS(fetchable_files_restricted_id->perform_action(action), FetchActionError);
            }
        }

        bool repeatable() const
        {
            return false;
        }
    } test_e_repository_fetch;

    struct ERepositoryManifestCheckTest : TestCase
    {
        ERepositoryManifestCheckTest() : TestCase("manifest_check") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "e_repository_TEST_dir/repo11");
            keys->insert("profiles", "e_repository_TEST_dir/repo11/profiles/profile");
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(
                        &env, keys));
            env.package_database()->add_repository(1, repo);

            FetchAction action(FetchActionOptions::create()
                    .fetch_unneeded(false)
                    .safe_resume(true)
                    );

            const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                            PackageDepSpec(parse_user_package_dep_spec("category/package",
                                    UserPackageDepSpecOptions()))), qo_order_by_version)->last());
            TEST_CHECK(id);
            id->perform_action(action);
        }
    } test_e_repository_manifest_check;

    struct ERepositoryInstallEAPI0Test : TestCase
    {
        ERepositoryInstallEAPI0Test() : TestCase("install_eapi_0") { }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TestEnvironment env;

            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
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
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

            tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("installed_virtuals")(&env, iv_keys));
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("virtuals")(&env,
                        tr1::shared_ptr<Map<std::string, std::string> >()));

            InstallAction action(InstallActionOptions::create()
                    .debug_build(iado_none)
                    .checks(iaco_default)
                    .no_config_protect(false)
                    .destination(installed_repo)
                    );

            {
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=virtual/virtual-pretend-installed-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
            }

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("econf source 0", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataStringKey>(**id->find_metadata("EAPI"))->value(), "0");
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
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
            return 300;
        }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
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
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            env.package_database()->add_repository(2, installed_repo);

            InstallAction action(InstallActionOptions::create()
                    .debug_build(iado_none)
                    .checks(iaco_default)
                    .no_config_protect(false)
                    .destination(installed_repo)
                    );

            {
                TestMessageSuffix suffix("econf source 1", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/econf-source-1",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_EQUAL(visitor_cast<const MetadataStringKey>(**id->find_metadata("EAPI"))->value(), "1");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_eapi_1;

    struct ERepositoryInstallExheres0Test : TestCase
    {
        ERepositoryInstallExheres0Test() : TestCase("install_exheres_0") { }

        unsigned max_run_time() const
        {
            return 300;
        }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
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
            tr1::shared_ptr<ERepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<FakeInstalledRepository> installed_repo(new FakeInstalledRepository(&env, RepositoryName("installed")));
            installed_repo->add_version("cat", "pretend-installed", "0")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            installed_repo->add_version("cat", "pretend-installed", "1")->provide_key()->set_from_string("virtual/virtual-pretend-installed");
            env.package_database()->add_repository(2, installed_repo);

            tr1::shared_ptr<Map<std::string, std::string> > iv_keys(new Map<std::string, std::string>);
            iv_keys->insert("root", "/");
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("installed_virtuals")(&env, iv_keys));
            env.package_database()->add_repository(-2, RepositoryMaker::get_instance()->find_maker("virtuals")(&env,
                        tr1::shared_ptr<Map<std::string, std::string> >()));

            InstallAction action(InstallActionOptions::create()
                    .debug_build(iado_none)
                    .checks(iaco_default)
                    .no_config_protect(false)
                    .destination(installed_repo)
                    );

            {
                TestMessageSuffix suffix("in-ebuild die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-ebuild-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("in-subshell die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/in-subshell-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("success", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/success",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("unpack die", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unpack-die",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("emake fail", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/emake-fail",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("best version", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/best-version-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("has version", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/has-version-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("match", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/match-0",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK_THROWS(id->perform_action(action), InstallActionError);
            }

            {
                TestMessageSuffix suffix("ever", true);
                const tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat/ever-1.3",
                                        UserPackageDepSpecOptions()))), qo_require_exactly_one)->last());
                TEST_CHECK(id);
                TEST_CHECK(id->short_description_key());
                TEST_CHECK_EQUAL(id->short_description_key()->value(), "The Description");
                id->perform_action(action);
            }
        }
    } test_e_repository_install_exheres_0;
}

