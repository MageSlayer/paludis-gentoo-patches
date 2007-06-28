/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/repositories/e/portage_repository.hh>
#include <paludis/repositories/e/make_ebuild_repository.hh>
#include <paludis/util/collection_concrete.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/util/system.hh>
#include <paludis/util/visitor-impl.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/eapi.hh>
#include <paludis/package_id.hh>
#include <paludis/metadata_key.hh>
#include <paludis/query.hh>
#include <paludis/dep_spec_pretty_printer.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for PortageRepository.
 *
 */

namespace test_cases
{
    /**
     * \test Test PortageRepository repository names.
     *
     */
    struct PortageRepositoryRepoNameTest : TestCase
    {
        PortageRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "test-repo-1");
        }
    } test_portage_repository_repo_name;

    /**
     * \test Test PortageRepository repository with no names.
     *
     */
    struct PortageRepositoryNoRepoNameTest : TestCase
    {
        PortageRepositoryNoRepoNameTest() : TestCase("no repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo2");
            keys->insert("profiles", "portage_repository_TEST_dir/repo2/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo2");
        }
    } test_portage_repository_no_repo_name;

    /**
     * \test Test PortageRepository repository empty names.
     *
     */
    struct PortageRepositoryEmptyRepoNameTest : TestCase
    {
        PortageRepositoryEmptyRepoNameTest() : TestCase("empty repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo3");
            keys->insert("profiles", "portage_repository_TEST_dir/repo3/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "x-repo3");
        }
    } test_portage_repository_empty_repo_name;

    /**
     * \test Test PortageRepository repository has_category_named.
     *
     */
    struct PortageRepositoryHasCategoryNamedTest : TestCase
    {
        PortageRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
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
    } test_portage_repository_has_category_named;

    /**
     * \test Test PortageRepository category_names.
     *
     */
    struct PortageRepositoryCategoryNamesTest : TestCase
    {
        PortageRepositoryCategoryNamesTest() : TestCase("category names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo1");
            keys->insert("profiles", "portage_repository_TEST_dir/repo1/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const CategoryNamePartCollection> c(repo->category_names());
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-one")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-two")));
                TEST_CHECK(c->end() != c->find(CategoryNamePart("cat-three")));
                TEST_CHECK(c->end() == c->find(CategoryNamePart("cat-four")));
                TEST_CHECK_EQUAL(3, std::distance(c->begin(), c->end()));
            }
        }
    } test_portage_repository_category_names;

    /**
     * \test Test PortageRepository has_package_named.
     *
     */
    struct PortageRepositoryHasPackageNamedTest : TestCase
    {
        PortageRepositoryHasPackageNamedTest() : TestCase("has package named") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profiles", "portage_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
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
    } test_portage_repository_has_package_named;

    /**
     * \test Test PortageRepository has_package_named cached.
     *
     */
    struct PortageRepositoryHasPackageNamedCachedTest : TestCase
    {
        PortageRepositoryHasPackageNamedCachedTest() : TestCase("has package named cached") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profiles", "portage_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
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
    } test_portage_repository_has_package_named_cached;

    /**
     * \test Test PortageRepository package_names.
     *
     */
    struct PortageRepositoryPackageNamesTest : TestCase
    {
        PortageRepositoryPackageNamesTest() : TestCase("package names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profiles", "portage_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const QualifiedPackageNameCollection> names;

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
    } test_portage_repository_package_names;

    /**
     * \test Test PortageRepository bad package names.
     *
     */
    struct PortageRepositoryBadPackageNamesTest : TestCase
    {
        PortageRepositoryBadPackageNamesTest() : TestCase("bad package names") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo5");
            keys->insert("profiles", "portage_repository_TEST_dir/repo5/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
                        &env, keys));

            tr1::shared_ptr<const QualifiedPackageNameCollection> names;

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                names = repo->package_names(CategoryNamePart("cat-one"));
                TEST_CHECK(! names->empty());
                TEST_CHECK(names->end() != names->find(QualifiedPackageName("cat-one/pkg-one")));
                TEST_CHECK_EQUAL(1, std::distance(names->begin(), names->end()));
            }
        }
    } test_portage_repository_bad_package_names;

    struct PortageRepositoryPackageIDTest : TestCase
    {
        PortageRepositoryPackageIDTest() : TestCase("package_ids") { }

        void run()
        {
            using namespace tr1::placeholders;

            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo4");
            keys->insert("profiles", "portage_repository_TEST_dir/repo4/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
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
    } test_portage_repository_versions;

    /**
     * \test Test PortageRepository duff versions.
     *
     */
    struct PortageRepositoryDuffVersionsTest : TestCase
    {
        PortageRepositoryDuffVersionsTest() : TestCase("duff versions") { }

        void run()
        {
            using namespace tr1::placeholders;

            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo8");
            keys->insert("profiles", "portage_repository_TEST_dir/repo8/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(
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
    } test_portage_repository_duff_versions;

    /**
     * \test Test PortageRepository cached metadata.
     *
     */
    struct PortageRepositoryMetadataCachedTest : TestCase
    {
        PortageRepositoryMetadataCachedTest() : TestCase("metadata cached") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo6");
            keys->insert("profiles", "portage_repository_TEST_dir/repo6/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);
                tr1::shared_ptr<const PackageID> id(*env.package_database()->query(query::Matches(
                                PackageDepSpec("=cat-one/pkg-one-1", pds_pm_unspecific)), qo_require_exactly_one)->begin());

                TEST_CHECK(id->short_description_key());
                TEST_CHECK_EQUAL(id->short_description_key()->value(), "the-description");
            }
        }
    } test_portage_repository_metadata_cached;

    /**
     * \test Test PortageRepository uncached metadata.
     *
     */
    struct PortageRepositoryMetadataUncachedTest : TestCase
    {
        PortageRepositoryMetadataUncachedTest() : TestCase("metadata uncached") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        void run()
        {
            for (int opass = 1 ; opass <= 3 ; ++opass)
            {
                TestMessageSuffix opass_suffix("opass=" + stringify(opass), true);

                TestEnvironment env;
                tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                        new AssociativeCollection<std::string, std::string>::Concrete);
                keys->insert("format", "ebuild");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", "portage_repository_TEST_dir/repo7/metadata/cache");
                keys->insert("location", "portage_repository_TEST_dir/repo7");
                keys->insert("profiles", "portage_repository_TEST_dir/repo7/profiles/profile");
                tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(&env, keys));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                    PackageDepSpec("=cat-one/pkg-one-1", pds_pm_unspecific)), qo_require_exactly_one)->begin());

                    TEST_CHECK(id1->eapi()->supported);
                    TEST_CHECK(id1->short_description_key());
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Description");
                    TEST_CHECK_EQUAL(id1->eapi()->name, "0");
                    DepSpecPrettyPrinter pd(0, false);
                    TEST_CHECK(id1->build_dependencies_key());
                    id1->build_dependencies_key()->value()->accept(pd);
                    TEST_CHECK_STRINGIFY_EQUAL(pd, "foo/bar");
                    DepSpecPrettyPrinter pr(0, false);
                    TEST_CHECK(id1->run_dependencies_key());
                    id1->run_dependencies_key()->value()->accept(pr);
                    TEST_CHECK_STRINGIFY_EQUAL(pr, "foo/bar");

                    tr1::shared_ptr<const PackageID> id2(*env.package_database()->query(query::Matches(
                                    PackageDepSpec("=cat-one/pkg-one-2", pds_pm_unspecific)), qo_require_exactly_one)->begin());

                    TEST_CHECK(id2->eapi()->supported);
                    TEST_CHECK(id2->short_description_key());
                    TEST_CHECK_EQUAL(id2->short_description_key()->value(), "dquote \" squote ' backslash \\ dollar $");
                    TEST_CHECK_EQUAL(id2->eapi()->name, "0");
                    DepSpecPrettyPrinter pd2(0, false);
                    TEST_CHECK(id2->build_dependencies_key());
                    id2->build_dependencies_key()->value()->accept(pd2);
                    TEST_CHECK_STRINGIFY_EQUAL(pd2, "foo/bar bar/baz");
                    DepSpecPrettyPrinter pr2(0, false);
                    TEST_CHECK(id2->run_dependencies_key());
                    id2->run_dependencies_key()->value()->accept(pr2);
                    TEST_CHECK_STRINGIFY_EQUAL(pr2, "foo/bar");
                }
            }
        }
    } test_portage_repository_metadata_uncached;

    /**
     * \test Test PortageRepository unparsable metadata.
     *
     */
    struct PortageRepositoryMetadataUnparsableTest : TestCase
    {
        PortageRepositoryMetadataUnparsableTest() : TestCase("metadata unparsable") { }

        bool skip() const
        {
            return ! getenv_with_default("SANDBOX_ON", "").empty();
        }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo7");
            keys->insert("profiles", "portage_repository_TEST_dir/repo7/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository( &env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageID> id1(*env.package_database()->query(query::Matches(
                                PackageDepSpec("=cat-one/pkg-two-1", pds_pm_unspecific)), qo_require_exactly_one)->begin());

                TEST_CHECK_EQUAL(id1->eapi()->name, "UNKNOWN");
                TEST_CHECK(! id1->eapi()->supported);
                TEST_CHECK(! id1->short_description_key());
            }
        }
    } test_portage_repository_metadata_unparsable;

    /**
     * \test Test PortageRepository query_use and query_use_mask functions.
     *
     */
    struct PortageRepositoryQueryUseTest : TestCase
    {
        PortageRepositoryQueryUseTest() : TestCase("USE query") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo9");
            keys->insert("profiles", "portage_repository_TEST_dir/repo9/profiles/profile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                tr1::shared_ptr<const PackageID> p1(*env.package_database()->query(query::Matches(PackageDepSpec(
                                    "=cat-one/pkg-one-1", pds_pm_unspecific)), qo_require_exactly_one)->begin());
                tr1::shared_ptr<const PackageID> p2(*env.package_database()->query(query::Matches(PackageDepSpec(
                                    "=cat-two/pkg-two-1", pds_pm_unspecific)), qo_require_exactly_one)->begin());
                tr1::shared_ptr<const PackageID> p4(*env.package_database()->query(query::Matches(PackageDepSpec(
                                    "=cat-one/pkg-one-2", pds_pm_unspecific)), qo_require_exactly_one)->begin());

                TEST_CHECK(repo->query_use(UseFlagName("flag1"), *p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag2"), *p1) == use_disabled);
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag2"), *p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), *p2));
                TEST_CHECK(! repo->query_use_mask(UseFlagName("flag3"), *p1));
                TEST_CHECK(repo->query_use_mask(UseFlagName("flag3"), *p4));
                TEST_CHECK(repo->query_use(UseFlagName("flag3"), *p1) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), *p2) == use_enabled);
                TEST_CHECK(repo->query_use(UseFlagName("flag5"), *p1) == use_unspecified);
            }
        }
    } test_portage_repository_query_use;

    /**
     * \test Test PortageRepository query_profile_masks functions.
     *
     */
    struct PortageRepositoryQueryProfileMasksTest : TestCase
    {
        PortageRepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<AssociativeCollection<std::string, std::string> > keys(
                    new AssociativeCollection<std::string, std::string>::Concrete);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", "portage_repository_TEST_dir/repo10");
            keys->insert("profiles", "portage_repository_TEST_dir/repo10/profiles/profile/subprofile");
            tr1::shared_ptr<PortageRepository> repo(make_ebuild_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK(repo->query_profile_masks(**env.package_database()->query(query::Matches(PackageDepSpec(
                                        "=cat/masked-0", pds_pm_unspecific)), qo_require_exactly_one)->begin()));
                TEST_CHECK(! repo->query_profile_masks(**env.package_database()->query(query::Matches(PackageDepSpec(
                                        "=cat/not_masked-0", pds_pm_unspecific)), qo_require_exactly_one)->begin()));
                TEST_CHECK(! repo->query_profile_masks(**env.package_database()->query(query::Matches(PackageDepSpec(
                                        "=cat/was_masked-0", pds_pm_unspecific)), qo_require_exactly_one)->begin()));
            }
        }
    } test_portage_repository_query_profile_masks;
}

