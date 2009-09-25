/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Fernando J. Pereda
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

#include "manifest.hh"
#include <paludis/qa.hh>
#include <paludis/util/system.hh>
#include <paludis/util/fd_holder.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/map.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/repositories/e/e_repository.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace paludis;
using namespace paludis::erepository;
using namespace test;

namespace
{
    struct TestReporter :
        QAReporter
    {
        unsigned count;

        TestReporter() :
            count(0)
        {
        }

        void message(const QAMessage &)
        {
            ++count;
        }

        void status(const std::string &)
        {
        }
    };

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
    struct GPGCheckTest : TestCase
    {
        GPGCheckTest() : TestCase("signed Manifest") { }

        bool skip() const
        {
            return (0 != run_command("gpg --help >/dev/null 2>/dev/null"));
        }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/not-signed");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 1u);

        }
    } test_gpg_check;

    struct ManifestGoodTest : TestCase
    {
        ManifestGoodTest() : TestCase("Manifest good") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/good");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 1u);
        }
    } test_manifest_good;

    struct ManifestBadTypeTest : TestCase
    {
        ManifestBadTypeTest() : TestCase("Manifest bad type") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/bad-type");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 3u);
        }
    } test_manifest_bad_type;

    struct ManifestBadSizeTest : TestCase
    {
        ManifestBadSizeTest() : TestCase("Manifest bad size") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/bad-size");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 2u);
        }
    } test_manifest_bad_size;

    struct ManifestBadHashTest : TestCase
    {
        ManifestBadHashTest() : TestCase("Manifest bad hash") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/bad-hash");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 5u);
        }
    } test_manifest_bad_hash;

    struct ManifestMissingFileTest : TestCase
    {
        ManifestMissingFileTest() : TestCase("Manifest missing file") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/missing");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 2u);
        }
    } test_manifest_missing_file;

    struct ManifestStrayFileTest : TestCase
    {
        ManifestStrayFileTest() : TestCase("Manifest stray file") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/stray");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 2u);
        }
    } test_manifest_stray_file;

    struct ManifestUnusedDistfileTest : TestCase
    {
        ManifestUnusedDistfileTest() : TestCase("Manifest unused distfile file") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/unused-distfile");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 2u);
        }
    } test_manifest_unused_distfile;

    struct ManifestUndigestedDistfileTest : TestCase
    {
        ManifestUndigestedDistfileTest() : TestCase("Manifest undigested distfile file") { }

        void run()
        {
            TestEnvironment env;
            std::tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "ebuild");
            keys->insert("names_cache", "/var/empty");
            keys->insert("location", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1"));
            keys->insert("profiles", stringify(FSEntry::cwd() / "manifest_TEST_dir/repo1/profiles/test"));
            std::tr1::shared_ptr<ERepository> repo(std::tr1::static_pointer_cast<ERepository>(ERepository::repository_factory_create(&env,
                        std::tr1::bind(from_keys, keys, std::tr1::placeholders::_1))));
            env.package_database()->add_repository(1, repo);

            QualifiedPackageName qpn("cat/undigested-distfile");
            FSEntry dir(repo->layout()->package_directory(qpn));

            TestReporter r;
            TEST_CHECK(manifest_check(r, dir, repo, qpn, "manifest"));
            TEST_CHECK_EQUAL(r.count, 2u);
        }
    } test_manifest_undigested_distfile;
}

