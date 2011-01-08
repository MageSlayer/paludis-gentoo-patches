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
#include <paludis/util/simple_visitor_cast.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_named_values.hh>
#include <paludis/util/set.hh>
#include <paludis/util/fs_stat.hh>
#include <paludis/util/make_null_shared_ptr.hh>
#include <paludis/standard_output_manager.hh>
#include <paludis/util/safe_ifstream.hh>
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

#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <algorithm>
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
    struct ERepositoryRepoNameTest : TestCase
    {
        ERepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo1/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo2"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo2/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo3"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo3/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
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
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::shared_ptr<const CategoryNamePartSet> c(repo->category_names());
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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));

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
            using namespace std::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo4/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1", VersionSpecOptions()))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1", VersionSpecOptions()))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("2", VersionSpecOptions()))));

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
            using namespace std::placeholders;

            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo8"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo8/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                std::shared_ptr<const PackageIDSequence> versions;

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-one"));
                TEST_CHECK(! versions->empty());
                TEST_CHECK_EQUAL(2, std::distance(versions->begin(), versions->end()));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1", VersionSpecOptions()))));
                TEST_CHECK(indirect_iterator(versions->end()) != std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("1.1-r1", VersionSpecOptions()))));
                TEST_CHECK(indirect_iterator(versions->end()) == std::find_if(
                            indirect_iterator(versions->begin()), indirect_iterator(versions->end()),
                            std::bind(std::equal_to<VersionSpec>(), std::bind(std::mem_fn(&PackageID::version), _1), VersionSpec("2", VersionSpecOptions()))));

                versions = repo->package_ids(QualifiedPackageName("cat-one/pkg-neither"));
                TEST_CHECK(versions->empty());
                TEST_CHECK_EQUAL(0, std::distance(versions->begin(), versions->end()));
            }
        }
    } test_e_repository_duff_versions;

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
                std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
                keys->insert("format", "e");
                keys->insert("names_cache", "/var/empty");
                keys->insert("write_cache", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/metadata/cache"));
                keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7"));
                keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
                keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
                std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1)));
                env.package_database()->add_repository(1, repo);

                for (int pass = 1 ; pass <= 3 ; ++pass)
                {
                    TestMessageSuffix pass_suffix("pass=" + stringify(pass), true);

                    const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin());

                    TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                    TEST_CHECK(simple_visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI")));
                    TEST_CHECK_EQUAL(simple_visitor_cast<const MetadataValueKey<std::string> >(**id1->find_metadata("EAPI"))->value(), "0");
                    TEST_CHECK(bool(id1->short_description_key()));
                    TEST_CHECK_EQUAL(id1->short_description_key()->value(), "The Description");
                    UnformattedPrettyPrinter ff;
                    erepository::SpecTreePrettyPrinter pd(ff, { });
                    TEST_CHECK(bool(id1->build_dependencies_key()));
                    id1->build_dependencies_key()->value()->top()->accept(pd);
                    TEST_CHECK_STRINGIFY_EQUAL(pd, "foo/bar");
                    erepository::SpecTreePrettyPrinter pr(ff, { });
                    TEST_CHECK(bool(id1->run_dependencies_key()));
                    id1->run_dependencies_key()->value()->top()->accept(pr);
                    TEST_CHECK_STRINGIFY_EQUAL(pr, "foo/bar");

                    const std::shared_ptr<const PackageID> id2(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin());

                    TEST_CHECK(id2->end_metadata() != id2->find_metadata("EAPI"));
                    TEST_CHECK(bool(id2->short_description_key()));
                    TEST_CHECK_EQUAL(id2->short_description_key()->value(), "dquote \" squote ' backslash \\ dollar $");
                    erepository::SpecTreePrettyPrinter pd2(ff, { });
                    TEST_CHECK(bool(id2->build_dependencies_key()));
                    id2->build_dependencies_key()->value()->top()->accept(pd2);
                    TEST_CHECK_STRINGIFY_EQUAL(pd2, "foo/bar bar/baz");
                    erepository::SpecTreePrettyPrinter pr2(ff, { });
                    TEST_CHECK(bool(id2->run_dependencies_key()));
                    id2->run_dependencies_key()->value()->top()->accept(pr2);
                    TEST_CHECK_STRINGIFY_EQUAL(pr2, "foo/bar");

                    const std::shared_ptr<const PackageID> id3(*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-3",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin());

                    TEST_CHECK(id3->end_metadata() != id3->find_metadata("EAPI"));
                    TEST_CHECK(bool(id3->short_description_key()));
                    TEST_CHECK_EQUAL(id3->short_description_key()->value(), "This is the short description");
                    TEST_CHECK(bool(id3->long_description_key()));
                    TEST_CHECK_EQUAL(id3->long_description_key()->value(), "This is the long description");
                }
            }
        }
    } test_e_repository_metadata_uncached;

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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo7/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::shared_ptr<const PackageID> id1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-two-1",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());

                TEST_CHECK(id1->end_metadata() != id1->find_metadata("EAPI"));
                TEST_CHECK_EQUAL(std::static_pointer_cast<const erepository::ERepositoryID>(id1)->eapi()->name(), "UNKNOWN");
                TEST_CHECK(! id1->short_description_key());
            }
        }
    } test_e_repository_metadata_unparsable;

    struct ERepositoryQueryUseTest : TestCase
    {
        ERepositoryQueryUseTest() : TestCase("USE query") { }

        void test_choice(const std::shared_ptr<const PackageID> & p, const std::string & n, bool enabled, bool enabled_by_default, bool locked, const std::string & u = "")
        {
            TestMessageSuffix s(stringify(*p) + "[" + n + "]", true);
            std::shared_ptr<const ChoiceValue> choice(p->choices_key()->value()->find_by_name_with_prefix(ChoiceNameWithPrefix(n)));
            TEST_CHECK(bool(choice));
            TEST_CHECK_EQUAL(choice->unprefixed_name(), UnprefixedChoiceName(u.empty() ? n : u));
            TEST_CHECK_EQUAL(choice->enabled(), enabled);
            TEST_CHECK_EQUAL(choice->enabled_by_default(), enabled_by_default);
            TEST_CHECK_EQUAL(choice->locked(), locked);
        }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo9/profiles/child"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                const std::shared_ptr<const PackageID> p1(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-1",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                const std::shared_ptr<const PackageID> p2(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-two/pkg-two-1",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());
                const std::shared_ptr<const PackageID> p4(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=cat-one/pkg-one-2",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin());

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
    } test_e_repository_query_use;

    struct ERepositoryRepositoryMasksTest : TestCase
    {
        ERepositoryRepositoryMasksTest() : TestCase("repository masks") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");

            std::shared_ptr<Map<std::string, std::string> > keys18(std::make_shared<Map<std::string, std::string>>());
            keys18->insert("format", "e");
            keys18->insert("names_cache", "/var/empty");
            keys18->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo18"));
            keys18->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo18/profiles/profile"));
            keys18->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo18(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys18, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo18);

            std::shared_ptr<Map<std::string, std::string> > keys19(std::make_shared<Map<std::string, std::string>>());
            keys19->insert("format", "e");
            keys19->insert("names_cache", "/var/empty");
            keys19->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo19"));
            keys19->insert("master_repository", "test-repo-18");
            keys19->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo19(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys19, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo19);

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-18",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-18",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-18",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-18",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());

            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-1::test-repo-19",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-2::test-repo-19",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-3::test-repo-19",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("=category/package-4::test-repo-19",
                                        &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
        }
    } test_e_repository_repository_masks;

    struct ERepositoryQueryProfileMasksTest : TestCase
    {
        ERepositoryQueryProfileMasksTest() : TestCase("profiles package.mask") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo10"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo10/profiles/profile/subprofile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<Repository> repo(ERepository::repository_factory_create(&env,
                        std::bind(from_keys, keys, std::placeholders::_1)));
            env.package_database()->add_repository(1, repo);

            for (int pass = 1 ; pass <= 2 ; ++pass)
            {
                TestMessageSuffix pass_suffix(stringify(pass), true);

                TEST_CHECK((*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/masked-0",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/was_masked-0",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
                TEST_CHECK(! (*env[selection::RequireExactlyOne(generator::Matches(
                                    PackageDepSpec(parse_user_package_dep_spec("=cat/not_masked-0",
                                            &env, { })), make_null_shared_ptr(), { }))]->begin())->masked());
            }
        }
    } test_e_repository_query_profile_masks;

    struct ERepositoryVirtualsTest : TestCase
    {
        ERepositoryVirtualsTest() : TestCase("virtuals") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo15"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo15/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            bool has_one(false), has_two(false);
            int count(0);

            std::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> seq(repo->virtual_packages());
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify(it->virtual_name()))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/two");
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-two/pkg-two");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK_EQUAL(count, 2);
        }
    } test_e_repository_virtuals;

    struct ERepositoryVirtuals2Test : TestCase
    {
        ERepositoryVirtuals2Test() : TestCase("virtuals 2") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo15"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo15/profiles/profile/subprofile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            bool has_one(false), has_two(false), has_three(false);
            int count(0);

            std::shared_ptr<const RepositoryVirtualsInterface::VirtualsSequence> seq(repo->virtual_packages());
            for (RepositoryVirtualsInterface::VirtualsSequence::ConstIterator it(seq->begin()),
                    it_end(seq->end()); it_end != it; ++it, ++count)
                if ("virtual/one" == stringify(it->virtual_name()))
                {
                    has_one = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-two/pkg-two");
                }
                else if ("virtual/two" == stringify(it->virtual_name()))
                {
                    has_two = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-one/pkg-one");
                }
                else
                {
                    TEST_CHECK_STRINGIFY_EQUAL(it->virtual_name(), "virtual/three");
                    has_three = true;
                    TEST_CHECK_STRINGIFY_EQUAL(*it->provided_by_spec(), "cat-three/pkg-three");
                }

            TEST_CHECK(has_one);
            TEST_CHECK(has_two);
            TEST_CHECK(has_three);
            TEST_CHECK_EQUAL(count, 3);
        }
    } test_e_repository_virtuals_2;

    struct ERepositoryManifestTest : TestCase
    {
        ERepositoryManifestTest() : TestCase("manifest2") { }

        void run()
        {
            TestEnvironment env;
            env.set_paludis_command("/bin/false");
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1))));
            env.package_database()->add_repository(1, repo);
            repo->make_manifest(QualifiedPackageName("category/package"));

            std::multiset<std::string> made_manifest, reference_manifest;
            SafeIFStream made_manifest_stream(FSPath("e_repository_TEST_dir/repo11/category/package/Manifest")),
                reference_manifest_stream(FSPath("e_repository_TEST_dir/repo11/Manifest_correct"));

            std::string line;

            while ( getline(made_manifest_stream, line) )
                made_manifest.insert(line);
            while ( getline(reference_manifest_stream, line) )
                reference_manifest.insert(line);

            TEST_CHECK(made_manifest == reference_manifest);

            TEST_CHECK_THROWS(repo->make_manifest(QualifiedPackageName("category/package-b")), MissingDistfileError);
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
            env.package_database()->add_repository(1, repo);

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
                TestMessageSuffix suffix("no files", true);
                const std::shared_ptr<const PackageID> no_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(no_files_id));
                TEST_CHECK(bool(no_files_id->short_description_key()));
                TEST_CHECK_EQUAL(no_files_id->short_description_key()->value(), "The Short Description");
                no_files_id->perform_action(action);
            }

            {
                TestMessageSuffix suffix("fetched files", true);
                const std::shared_ptr<const PackageID> fetched_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(fetched_files_id));
                TEST_CHECK((FSPath("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").stat().is_regular_file());
                fetched_files_id->perform_action(action);
                TEST_CHECK((FSPath("e_repository_TEST_dir") / "distdir" / "already-fetched.txt").stat().is_regular_file());
            }

            {
                TestMessageSuffix suffix("fetchable files", true);
                TEST_CHECK(! (FSPath("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").stat().is_regular_file());
                const std::shared_ptr<const PackageID> fetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(fetchable_files_id));
                fetchable_files_id->perform_action(action);
                TEST_CHECK((FSPath("e_repository_TEST_dir") / "distdir" / "fetchable-1.txt").stat().is_regular_file());
            }

            {
                TestMessageSuffix suffix("arrow files", true);
                TEST_CHECK(! (FSPath("e_repository_TEST_dir") / "distdir" / "arrowed.txt").stat().is_regular_file());
                const std::shared_ptr<const PackageID> arrow_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/arrow-files",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(arrow_files_id));
                arrow_files_id->perform_action(action);
                TEST_CHECK((FSPath("e_repository_TEST_dir") / "distdir" / "arrowed.txt").stat().is_regular_file());
            }

            {
                TestMessageSuffix suffix("unfetchable files", true);
                const std::shared_ptr<const PackageID> unfetchable_files_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/unfetchable-files",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(unfetchable_files_id));
                TEST_CHECK_THROWS(unfetchable_files_id->perform_action(action), ActionFailedError);
            }

            {
                const std::shared_ptr<const PackageID> no_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/no-files-restricted",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(no_files_restricted_id));
                no_files_restricted_id->perform_action(action);
            }

            {
                const std::shared_ptr<const PackageID> fetched_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetched-files-restricted",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(fetched_files_restricted_id));
                fetched_files_restricted_id->perform_action(action);
            }

            {
                const std::shared_ptr<const PackageID> fetchable_files_restricted_id(*env[selection::RequireExactlyOne(generator::Matches(
                                PackageDepSpec(parse_user_package_dep_spec("cat/fetchable-files-restricted",
                                        &env, { })), make_null_shared_ptr(), { }))]->last());
                TEST_CHECK(bool(fetchable_files_restricted_id));
                TEST_CHECK_THROWS(fetchable_files_restricted_id->perform_action(action), ActionFailedError);
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
            std::shared_ptr<Map<std::string, std::string> > keys(std::make_shared<Map<std::string, std::string>>());
            keys->insert("format", "e");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11"));
            keys->insert("profiles", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "repo11/profiles/profile"));
            keys->insert("builddir", stringify(FSPath::cwd() / "e_repository_TEST_dir" / "build"));
            std::shared_ptr<ERepository> repo(std::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                            std::bind(from_keys, keys, std::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

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
                                    &env, { })), make_null_shared_ptr(), { }))]->last());
            TEST_CHECK(bool(id));
            repo->make_manifest(id->name());
            id->perform_action(action);
        }
    } test_e_repository_manifest_check;
}

