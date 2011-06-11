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
        global e, nce, repo, irepo
        e = EnvironmentFactory.instance.create("")
        nce = NoConfigEnvironment(repo_path)
        repo = e.fetch_repository("testrepo")
        irepo = e.fetch_repository("installed")

    def test_01_fetch(self):
        self.assert_(isinstance(repo, Repository))
        self.assert_(isinstance(irepo, Repository))

    def test_02_create_error(self):
        self.assertRaises(Exception, Repository)

    def test_03_name(self):
        self.assertEquals(str(repo.name), "testrepo")
        self.assertEquals(str(irepo.name), "installed")

    def test_05_has_category_named(self):
        self.assert_(repo.has_category_named("foo", []))
        self.assert_(not repo.has_category_named("bar", []))

    def test_06_has_package_named(self):
        self.assert_(repo.has_package_named("foo/bar", []))
        self.assert_(not repo.has_package_named("foo/foo", []))
        self.assert_(not repo.has_package_named("bar/foo", []))

    def test_07_package_ids(self):
        y = list(x.version for x in repo.package_ids("foo/bar", []));
        y.sort()
        z = [VersionSpec("1.0"), VersionSpec("2.0")]
        self.assertEquals(y, z)
        self.assertEquals(len(list(repo.package_ids("bar/baz", []))), 0)

    def test_08_category_names(self):
        self.assertEquals([str(x) for x in repo.category_names([])], ["foo", "foo1", "foo2", "foo3", "foo4"])

    def test_09_category_names_containing_package(self):
        self.assertEquals([str(x) for x in repo.category_names_containing_package("bar", [])],
                ["foo", "foo1", "foo2", "foo3", "foo4"])

    def test_10_package_names(self):
        for (i, qpn) in enumerate(repo.package_names("bar", [])):
            self.assertEquals(i, 0)
            self.assertEquals(str(qpn), "foo/bar")

    def test_11_some_ids_might_support_action(self):
        self.assert_(repo.some_ids_might_support_action(SupportsFetchActionTest()))
        self.assert_(not irepo.some_ids_might_support_action(SupportsFetchActionTest()))
        self.assert_(repo.some_ids_might_support_action(SupportsInstallActionTest()))
        self.assert_(not irepo.some_ids_might_support_action(SupportsInstallActionTest()))
        self.assert_(not repo.some_ids_might_support_action(SupportsUninstallActionTest()))
        self.assert_(irepo.some_ids_might_support_action(SupportsUninstallActionTest()))
        self.assert_(repo.some_ids_might_support_action(SupportsPretendActionTest()))
        self.assert_(not irepo.some_ids_might_support_action(SupportsPretendActionTest()))
        self.assert_(not repo.some_ids_might_support_action(SupportsConfigActionTest()))
        self.assert_(irepo.some_ids_might_support_action(SupportsConfigActionTest()))

    def test_12_keys(self):
        self.assert_(repo.location_key())
        self.assertEquals(repo.location_key().parse_value(), repo_path)
        self.assert_(repo.find_metadata("format"))
        self.assertEquals(repo.find_metadata("format").parse_value(), "e")
        self.assert_(not repo.find_metadata("asdf"))

class TestCase_02_RepositoryInterfaces(unittest.TestCase):
    def setUp(self):
        global e, nce, repo, irepo
        e = EnvironmentFactory.instance.create("")
        nce = NoConfigEnvironment(repo_path)
        repo = e.fetch_repository("testrepo")
        irepo = e.fetch_repository("installed")

    def test_06_environment_variable_interface(self):
        evi = repo.environment_variable_interface
        self.assert_(isinstance(evi, RepositoryEnvironmentVariableInterface))

    def test_09_virtuals_interface(self):
        vi = repo.virtuals_interface
        self.assert_(isinstance(vi, RepositoryVirtualsInterface))

    def test_10_destination_interface(self):
        di = irepo.destination_interface
        self.assert_(isinstance(di, RepositoryDestinationInterface))

class TestCase_03_FakeRepository(unittest.TestCase):
    def setUp(self):
        global e, f
        e = EnvironmentFactory.instance.create("")
        f = FakeRepository(e, "fake")

    def test_01_init(self):
        pass
        self.assertEquals(str(f.name), "fake")

    def test_02_init_bad(self):
        self.assertRaises(Exception, FakeRepository, e)
        self.assertRaises(Exception, FakeRepository, "foo", "foo")

    def test_03_add_category(self):
        self.assertEquals(list(f.category_names([])), [])

        f.add_category("cat-foo")
        self.assertEquals([str(x) for x in f.category_names([])], ["cat-foo"])
        f.add_category("cat-foo")
        self.assertEquals([str(x) for x in f.category_names([])], ["cat-foo"])
        f.add_category("cat-bar")
        self.assertEquals([str(x) for x in f.category_names([])], ["cat-bar", "cat-foo"])

    def test_04_add_category_bad(self):
        self.assertRaises(Exception, f.add_category, 1)
        self.assertRaises(Exception, f.add_category, "a", "b")

    def test_05_add_package(self):
        foo_1 = QualifiedPackageName("cat-foo/pkg1")
        foo_2 = QualifiedPackageName("cat-foo/pkg2")
        bar_1 = QualifiedPackageName("cat-bar/pkg1")

        f.add_category("cat-foo")
        self.assertEquals(list(f.package_names("cat-foo", [])), [])

        f.add_package(foo_1)
        self.assertEquals(list(f.package_names("cat-foo", [])), [foo_1])
        f.add_package(foo_1)
        self.assertEquals(list(f.package_names("cat-foo", [])), [foo_1])

        f.add_package(foo_2)
        self.assertEquals(list(f.package_names("cat-foo", [])), [foo_1, foo_2])

        f.add_package(bar_1)
        self.assertEquals(list(f.package_names("cat-bar", [])), [bar_1])

    def test_06_add_package_bad(self):
        self.assertRaises(Exception, f.add_category, 1)
        self.assertRaises(Exception, f.add_category, "a", "b")

    def test_07_add_version(self):
        f.add_package("cat-foo/pkg")
        self.assertEquals(list(f.package_ids("cat-foo/pkg", [])), [])

        pkg = f.add_version("cat-foo/pkg", VersionSpec("1"))
        self.assertEquals(list(f.package_ids("cat-foo/pkg", [])), [pkg])
        self.assertEquals(pkg.version, VersionSpec("1"))

        pkg2 = f.add_version("cat-foo/pkg", VersionSpec("2"))
        self.assertEquals(list(f.package_ids("cat-foo/pkg", [])), [pkg, pkg2])
        self.assertEquals(pkg2.version, VersionSpec("2"))

        pkg3 = f.add_version("cat-bar/pkg", VersionSpec("0"))
        self.assertEquals(list(f.package_ids("cat-bar/pkg", [])), [pkg3])

        self.assertEquals([str(x) for x in f.category_names([])], ["cat-bar", "cat-foo"])

    def test_08_add_version_bad(self):
        self.assertRaises(Exception, f.add_version, 1)
        self.assertRaises(Exception, f.add_version, "a", "b", "c")


if __name__ == "__main__":
    unittest.main()

