/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#include <paludis/environment_implementation.hh>
#include <paludis/package_database.hh>
#include <paludis/repositories/fake/fake_repository.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

using namespace paludis;
using namespace test;

namespace
{
    class EITestEnvironment :
        public EnvironmentImplementation
    {
        private:
            std::tr1::shared_ptr<PackageDatabase> _package_database;

        public:
            EITestEnvironment() :
                _package_database(new PackageDatabase(this))
            {
                std::tr1::shared_ptr<FakeRepository> repo(new FakeRepository(this, RepositoryName("repo")));
                _package_database->add_repository(1, std::tr1::shared_ptr<Repository>(repo));

                repo->add_version("foo", "one", "0")->ebuild_interface->keywords_string = "test foo";
                repo->add_version("foo", "two", "0")->ebuild_interface->keywords_string = "~test foo";
                repo->add_version("foo", "three", "0")->ebuild_interface->keywords_string = "-test foo";
                repo->add_version("foo", "four", "0")->ebuild_interface->keywords_string = "-* foo";
            }

            ~EITestEnvironment()
            {
            }

            std::tr1::shared_ptr<PackageDatabase> package_database()
            {
                return _package_database;
            }

            std::tr1::shared_ptr<const PackageDatabase> package_database() const
            {
                return _package_database;
            }

            std::string paludis_command() const
            {
                return "";
            }

            void set_paludis_command(const std::string &)
            {
            }

            bool accept_keywords(std::tr1::shared_ptr<const KeywordNameCollection> k, const PackageDatabaseEntry &) const
            {
                return k->end() != k->find(KeywordName("test")) || k->end() != k->find(KeywordName("*"));
            }
    };
}

namespace test_cases
{
    struct MaskReasonsTest : TestCase
    {
        MaskReasonsTest() : TestCase("mask reasons") { }

        void run()
        {
            EITestEnvironment env;

            MaskReasons m1(env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/one"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(! m1[mr_keyword]);

            MaskReasons m2(env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m2[mr_keyword]);
            m2 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(! m2[mr_keyword]);
            m2 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(! m2[mr_keyword]);

            MaskReasons m3(env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m3[mr_keyword]);
            m3 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(m3[mr_keyword]);
            m3 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(m3[mr_keyword]);

            MaskReasons m4(env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m4[mr_keyword]);
            m4 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(m4[mr_keyword]);
            m4 = env.mask_reasons(PackageDatabaseEntry(
                            QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(m4[mr_keyword]);
        }
    } test_mask_reasons;
}

