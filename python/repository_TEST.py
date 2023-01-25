#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA
#

import os

repo_path = os.path.join(os.getcwd(), "repository_TEST_dir/testrepo")
os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "repository_TEST_dir/home")

from paludis import *

# To check for QA easily
import paludis

import unittest

Log.instance.log_level = LogLevel.WARNING


class TestCase_01_Repository(unittest.TestCase):
    def setUp(self):
        global e, repo, irepo
        e = EnvironmentFactory.instance.create("")
        repo = e.fetch_repository("testrepo")
        irepo = e.fetch_repository("installed")

    def test_01_fetch(self):
        self.assertTrue(isinstance(repo, Repository))
        self.assertTrue(isinstance(irepo, Repository))

    def test_02_create_error(self):
        self.assertRaises(Exception, Repository)

    def test_03_name(self):
        self.assertEqual(str(repo.name), "testrepo")
        self.assertEqual(str(irepo.name), "installed")

    def test_05_has_category_named(self):
        self.assertTrue(repo.has_category_named("foo", []))
        self.assertTrue(not repo.has_category_named("bar", []))

    def test_06_has_package_named(self):
        self.assertTrue(repo.has_package_named("foo/bar", []))
        self.assertTrue(not repo.has_package_named("foo/foo", []))
        self.assertTrue(not repo.has_package_named("bar/foo", []))

    def test_07_package_ids(self):
        y = list(x.version for x in repo.package_ids("foo/bar", []))
        y.sort()
        z = [VersionSpec("1.0"), VersionSpec("2.0")]
        self.assertEqual(y, z)
        self.assertEqual(len(list(repo.package_ids("bar/baz", []))), 0)

    def test_08_category_names(self):
        self.assertEqual(
            [str(x) for x in repo.category_names([])],
            ["foo", "foo1", "foo2", "foo3", "foo4"],
        )

    def test_09_category_names_containing_package(self):
        self.assertEqual(
            [str(x) for x in repo.category_names_containing_package("bar", [])],
            ["foo", "foo1", "foo2", "foo3", "foo4"],
        )

    def test_10_package_names(self):
        for (i, qpn) in enumerate(repo.package_names("bar", [])):
            self.assertEqual(i, 0)
            self.assertEqual(str(qpn), "foo/bar")

    def test_11_some_ids_might_support_action(self):
        self.assertTrue(repo.some_ids_might_support_action(SupportsFetchActionTest()))
        self.assertTrue(
            not irepo.some_ids_might_support_action(SupportsFetchActionTest())
        )
        self.assertTrue(repo.some_ids_might_support_action(SupportsInstallActionTest()))
        self.assertTrue(
            not irepo.some_ids_might_support_action(SupportsInstallActionTest())
        )
        self.assertTrue(
            not repo.some_ids_might_support_action(SupportsUninstallActionTest())
        )
        self.assertTrue(
            irepo.some_ids_might_support_action(SupportsUninstallActionTest())
        )
        self.assertTrue(repo.some_ids_might_support_action(SupportsPretendActionTest()))
        self.assertTrue(
            not irepo.some_ids_might_support_action(SupportsPretendActionTest())
        )
        self.assertTrue(
            not repo.some_ids_might_support_action(SupportsConfigActionTest())
        )
        self.assertTrue(irepo.some_ids_might_support_action(SupportsConfigActionTest()))

    def test_12_keys(self):
        self.assertTrue(repo.location_key())
        self.assertEqual(repo.location_key().parse_value(), repo_path)
        self.assertTrue(repo.find_metadata("format"))
        self.assertEqual(repo.find_metadata("format").parse_value(), "e")
        self.assertTrue(not repo.find_metadata("asdf"))


class TestCase_02_RepositoryInterfaces(unittest.TestCase):
    def setUp(self):
        global e, repo, irepo
        e = EnvironmentFactory.instance.create("")
        repo = e.fetch_repository("testrepo")
        irepo = e.fetch_repository("installed")

    def test_06_environment_variable_interface(self):
        evi = repo.environment_variable_interface
        self.assertTrue(isinstance(evi, RepositoryEnvironmentVariableInterface))

    def test_10_destination_interface(self):
        di = irepo.destination_interface
        self.assertTrue(isinstance(di, RepositoryDestinationInterface))


class TestCase_03_FakeRepository(unittest.TestCase):
    def setUp(self):
        global e, f
        e = EnvironmentFactory.instance.create("")
        f = FakeRepository(e, "fake")

    def test_01_init(self):
        pass
        self.assertEqual(str(f.name), "fake")

    def test_02_init_bad(self):
        self.assertRaises(Exception, FakeRepository, e)
        self.assertRaises(Exception, FakeRepository, "foo", "foo")

    def test_03_add_category(self):
        self.assertEqual(list(f.category_names([])), [])

        f.add_category("cat-foo")
        self.assertEqual([str(x) for x in f.category_names([])], ["cat-foo"])
        f.add_category("cat-foo")
        self.assertEqual([str(x) for x in f.category_names([])], ["cat-foo"])
        f.add_category("cat-bar")
        self.assertEqual([str(x) for x in f.category_names([])], ["cat-bar", "cat-foo"])

    def test_04_add_category_bad(self):
        self.assertRaises(Exception, f.add_category, 1)
        self.assertRaises(Exception, f.add_category, "a", "b")

    def test_05_add_package(self):
        foo_1 = QualifiedPackageName("cat-foo/pkg1")
        foo_2 = QualifiedPackageName("cat-foo/pkg2")
        bar_1 = QualifiedPackageName("cat-bar/pkg1")

        f.add_category("cat-foo")
        self.assertEqual(list(f.package_names("cat-foo", [])), [])

        f.add_package(foo_1)
        self.assertEqual(list(f.package_names("cat-foo", [])), [foo_1])
        f.add_package(foo_1)
        self.assertEqual(list(f.package_names("cat-foo", [])), [foo_1])

        f.add_package(foo_2)
        self.assertEqual(list(f.package_names("cat-foo", [])), [foo_1, foo_2])

        f.add_package(bar_1)
        self.assertEqual(list(f.package_names("cat-bar", [])), [bar_1])

    def test_06_add_package_bad(self):
        self.assertRaises(Exception, f.add_category, 1)
        self.assertRaises(Exception, f.add_category, "a", "b")

    def test_07_add_version(self):
        f.add_package("cat-foo/pkg")
        self.assertEqual(list(f.package_ids("cat-foo/pkg", [])), [])

        pkg = f.add_version("cat-foo/pkg", VersionSpec("1"))
        self.assertEqual(list(f.package_ids("cat-foo/pkg", [])), [pkg])
        self.assertEqual(pkg.version, VersionSpec("1"))

        pkg2 = f.add_version("cat-foo/pkg", VersionSpec("2"))
        self.assertEqual(list(f.package_ids("cat-foo/pkg", [])), [pkg, pkg2])
        self.assertEqual(pkg2.version, VersionSpec("2"))

        pkg3 = f.add_version("cat-bar/pkg", VersionSpec("0"))
        self.assertEqual(list(f.package_ids("cat-bar/pkg", [])), [pkg3])

        self.assertEqual([str(x) for x in f.category_names([])], ["cat-bar", "cat-foo"])

    def test_08_add_version_bad(self):
        self.assertRaises(Exception, f.add_version, 1)
        self.assertRaises(Exception, f.add_version, "a", "b", "c")


if __name__ == "__main__":
    unittest.main()
