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
#include <paludis/repositories/fake/fake_package_id.hh>
#include <paludis/util/tr1_functional.hh>
#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>
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
            tr1::shared_ptr<PackageDatabase> _package_database;

        public:
            EITestEnvironment() :
                _package_database(new PackageDatabase(this))
            {
                tr1::shared_ptr<FakeRepository> repo(new FakeRepository(this, RepositoryName("repo")));
                _package_database->add_repository(1, tr1::shared_ptr<Repository>(repo));

                repo->add_version("foo", "one", "0")->keywords_key()->set_from_string("test foo");
                repo->add_version("foo", "two", "0")->keywords_key()->set_from_string("~test foo");
                repo->add_version("foo", "three", "0")->keywords_key()->set_from_string("-test foo");
                repo->add_version("foo", "four", "0")->keywords_key()->set_from_string("-* foo");
            }

            ~EITestEnvironment()
            {
            }

            tr1::shared_ptr<PackageDatabase> package_database()
            {
                return _package_database;
            }

            tr1::shared_ptr<const PackageDatabase> package_database() const
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

            bool accept_keywords(tr1::shared_ptr<const KeywordNameCollection> k, const PackageID &) const
            {
                return k->end() != k->find(KeywordName("test")) || k->end() != k->find(KeywordName("*"));
            }

            const tr1::shared_ptr<const PackageID> fetch_package_id(const QualifiedPackageName & q,
                    const VersionSpec & v, const RepositoryName & r) const
            {
                using namespace tr1::placeholders;

                tr1::shared_ptr<const PackageIDSequence> ids(package_database()->fetch_repository(r)->package_ids(q));
                PackageIDSequence::Iterator i(std::find_if(ids->begin(), ids->end(),
                            tr1::bind(std::equal_to<VersionSpec>(), tr1::bind(tr1::mem_fn(&PackageID::version), _1), v)));
                if (i == ids->end())
                    throw NoSuchPackageError(stringify(q) + "-" + stringify(v) + "::" + stringify(r));
                return *i;
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

            MaskReasons m1(env.mask_reasons(*env.fetch_package_id(
                            QualifiedPackageName("foo/one"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(! m1[mr_keyword]);

            MaskReasons m2(env.mask_reasons(*env.fetch_package_id(
                            QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m2[mr_keyword]);
            m2 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(! m2[mr_keyword]);
            m2 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/two"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(! m2[mr_keyword]);

            MaskReasons m3(env.mask_reasons(*env.fetch_package_id(
                            QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m3[mr_keyword]);
            m3 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(m3[mr_keyword]);
            m3 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/three"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(m3[mr_keyword]);

            MaskReasons m4(env.mask_reasons(*env.fetch_package_id(
                            QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo"))));
            TEST_CHECK(m4[mr_keyword]);
            m4 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_tilde_keywords);
            TEST_CHECK(m4[mr_keyword]);
            m4 = env.mask_reasons(*env.fetch_package_id(
                        QualifiedPackageName("foo/four"), VersionSpec("0"), RepositoryName("repo")),
                    MaskReasonsOptions() + mro_override_unkeyworded);
            TEST_CHECK(m4[mr_keyword]);
        }
    } test_mask_reasons;
}

