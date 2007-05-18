#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2007 Piotr Jaroszyński <peper@gentoo.org>
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

repo = os.path.join(os.getcwd(), "repository_TEST_dir/testrepo")
os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "repository_TEST_dir/home")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING;

class TestCase_Repository(unittest.TestCase):
    def get_foo(self):
        self.e = EnvironmentMaker.instance.make_from_spec("")
        self.nce = NoConfigEnvironment(repo)
        self.db = self.e.package_database
        self.repo = self.db.fetch_repository("testrepo")
        self.irepo = self.db.fetch_repository("installed")

    def test_01_get(self):
        self.get_foo()

    def test_02_create_error(self):
        self.assertRaises(Exception, Repository)

    def test_03_name(self):
        self.get_foo()

        self.assertEquals(str(self.repo.name), "testrepo")
        self.assertEquals(str(self.irepo.name), "installed")

    def test_04_format(self):
        self.get_foo()

        self.assertEquals(str(self.repo.format), "ebuild")
        self.assertEquals(str(self.irepo.format), "vdb")

    def test_05_has_version(self):
        self.get_foo()

        self.assert_(self.repo.has_version("foo/bar", "1.0"))
        self.assert_(self.repo.has_version("foo/bar", "2.0"))
        self.assert_(not self.repo.has_version("foo/barbar", "1.0"))
        self.assert_(not self.repo.has_version("foo/bar", "3.0"))

    def test_06_has_category_named(self):
        self.get_foo()

        self.assert_(self.repo.has_category_named("foo"))
        self.assert_(not self.repo.has_category_named("bar"))

    def test_07_has_package_named(self):
        self.get_foo()

        self.assert_(self.repo.has_package_named("foo/bar"))
        self.assert_(not self.repo.has_package_named("foo/foo"))
        self.assert_(not self.repo.has_package_named("bar/foo"))

    def test_08_version_specs(self):
        self.get_foo()

#        self.assertEquals(list(self.repo.version_specs("foo/bar")),
#                [VersionSpec("1.0"), VersionSpec("2.0")]
#                )
#        self.assertEquals(len(list(self.repo.version_specs("bar/baz"))), 0)

    def test_09_category_names(self):
        self.get_foo()

        for (i, cat) in enumerate(self.repo.category_names):
            self.assertEquals(i, 0)
            self.assertEquals(str(cat), "foo")

    def test_10_category_names_containing_package(self):
        self.get_foo()

        for (i, cat) in enumerate(self.repo.category_names_containing_package("bar")):
            self.assertEquals(i, 0)
            self.assertEquals(str(cat), "foo")

    def test_11_package_names(self):
        self.get_foo()

        for (i, qpn) in enumerate(self.repo.package_names("bar")):
            self.assertEquals(i, 0)
            self.assertEquals(str(qpn), "foo/bar")

    def test_12_repository_info(self):
        self.get_foo()

        self.assert_(isinstance(self.repo.info(True), RepositoryInfo))
        self.assert_(isinstance(self.repo.info(False), RepositoryInfo))

        for (i, section) in enumerate(self.repo.info(False).sections):
            self.assert_(isinstance(section, RepositoryInfoSection))
            if i == 0:
                self.assertEquals(section.heading, "Configuration information")
                for (k, v) in section.kvs:
                    if k == "format":
                        self.assertEquals(v, "ebuild")

        for (i, section) in enumerate(self.irepo.info(False).sections):
            self.assert_(isinstance(section, RepositoryInfoSection))
            if i == 0:
                self.assertEquals(section.heading, "Configuration information")
                for (k, v) in section.kvs:
                    if k == "format":
                        self.assertEquals(v, "vdb")

    def test_13_version_metadata(self):
        self.get_foo()

        vm = self.repo.version_metadata("foo/bar", "1.0")
        self.assert_(isinstance(vm, VersionMetadata))


    def test_14_installable_interface(self):
        self.get_foo()

        ii = self.repo.installable_interface
        self.assert_(isinstance(ii, RepositoryInstallableInterface))

    def test_15_installed_interface(self):
        import datetime
        self.get_foo()

        ii = self.irepo.installed_interface

        self.assert_(isinstance(ii, RepositoryInstalledInterface))

        time = ii.installed_time("cat-one/pkg-one", "1")
        self.assert_(type(time) == datetime.datetime)

    def test_16_mask_interface(self):
        self.get_foo()

        mi = self.repo.mask_interface
        self.assert_(isinstance(mi, RepositoryMaskInterface))

        self.assert_(mi.query_profile_masks("foo1/bar", "1.0"))
        self.assert_(not mi.query_profile_masks("foo2/bar", "1.0"))
        self.assert_(mi.query_profile_masks("foo3/bar", "1.0"))
        self.assert_(not mi.query_profile_masks("foo4/bar", "1.0"))

    def test_17_sets_interface(self):
        self.get_foo()

        si = self.repo.sets_interface
        self.assert_(isinstance(si, RepositorySetsInterface))

    def test_18_syncable_interface(self):
        self.get_foo()

        si = self.repo.syncable_interface
        self.assert_(isinstance(si, RepositorySyncableInterface))

    def test_19_uninstallable_interface(self):
        self.get_foo()

        ui = self.irepo.uninstallable_interface
        self.assert_(isinstance(ui, RepositoryUninstallableInterface))

    def test_20_use_interface(self):
        self.get_foo()

        ui = self.repo.use_interface
        self.assert_(isinstance(ui, RepositoryUseInterface))

        p = PackageDatabaseEntry("foo/bar", "2.0", self.repo.name)

        self.assertEquals(ui.query_use("test1",p), UseFlagState.ENABLED)
        self.assertEquals(ui.query_use("test2",p), UseFlagState.DISABLED)
        self.assertEquals(ui.query_use("test3",p), UseFlagState.ENABLED)
        self.assertEquals(ui.query_use("test4",p), UseFlagState.UNSPECIFIED)
        self.assertEquals(ui.query_use("test5",p), UseFlagState.DISABLED)
        self.assertEquals(ui.query_use("test6",p), UseFlagState.ENABLED)
        self.assertEquals(ui.query_use("test7",p), UseFlagState.ENABLED)

        self.assert_(not ui.query_use_mask("test1", p))
        self.assert_(not ui.query_use_mask("test2", p))
        self.assert_(not ui.query_use_mask("test3", p))
        self.assert_(not ui.query_use_mask("test4", p))
        self.assert_(ui.query_use_mask("test5", p))
        self.assert_(not ui.query_use_mask("test6", p))
        self.assert_(not ui.query_use_mask("test7", p))

        self.assert_(not ui.query_use_force("test1", p))
        self.assert_(not ui.query_use_force("test2", p))
        self.assert_(not ui.query_use_force("test3", p))
        self.assert_(not ui.query_use_force("test4", p))
        self.assert_(not ui.query_use_force("test5", p))
        self.assert_(ui.query_use_force("test6", p))
        self.assert_(ui.query_use_force("test7", p))

        self.assert_(ui.describe_use_flag("test1", p), "A test use flag")

    def test_21_world_interface(self):
        self.get_foo()

        wi = self.irepo.world_interface
        self.assert_(isinstance(wi, RepositoryWorldInterface))

    def test_22_environment_variable_interface(self):
        self.get_foo()

        evi = self.repo.environment_variable_interface
        self.assert_(isinstance(evi, RepositoryEnvironmentVariableInterface))

    def test_23_mirrors_interface(self):
        self.get_foo()

        mi = self.repo.mirrors_interface
        self.assert_(isinstance(mi, RepositoryMirrorsInterface))

    def test_24_provides_interface(self):
        self.get_foo()

        pi = self.irepo.provides_interface
        self.assert_(isinstance(pi, RepositoryProvidesInterface))

    def test_25_virtuals_interface(self):
        self.get_foo()

        vi = self.repo.virtuals_interface
        self.assert_(isinstance(vi, RepositoryVirtualsInterface))

    def test_26_destination_interface(self):
        self.get_foo()

        di = self.irepo.destination_interface
        self.assert_(isinstance(di, RepositoryDestinationInterface))

    def test_27_contents_interface(self):
        self.get_foo()

        self.assert_(isinstance(self.irepo.contents_interface, RepositoryContentsInterface))

        contents = self.irepo.contents_interface.contents("cat-one/pkg-one", "1")

        for (i, entry) in enumerate(contents):
            if i == 0:
                self.assert_(isinstance(entry, ContentsDirEntry))
                self.assertEqual(str(entry), "//test")
                self.assertEqual(entry.name, "//test")
            elif i == 1:
                self.assert_(isinstance(entry, ContentsFileEntry))
                self.assertEqual(str(entry), "/test/test_file")
                self.assertEqual(entry.name, "/test/test_file")
            elif i == 2:
                self.assert_(isinstance(entry, ContentsSymEntry))
                self.assertEqual(str(entry), "/test/test_link -> /test/test_file")
                self.assertEqual(entry.name, "/test/test_link")
                self.assertEqual(entry.target, "/test/test_file")
            else:
                self.assertEqual("TOO MANY ENTRIES", "OK")

    def test_28_config_interface(self):
        self.get_foo()

        ci = self.irepo.config_interface
        self.assert_(isinstance(ci, RepositoryConfigInterface))

    def test_29_licenses_interface(self):
        self.get_foo()

        li = self.repo.licenses_interface
        self.assert_(isinstance(li, RepositoryLicensesInterface))

        self.assertEquals(os.path.realpath(li.license_exists("foo")), os.path.join(repo, "licenses/foo"))
        self.assertEquals(li.license_exists("bad"), None)

    def test_30_portage_interface(self):
        self.get_foo()

        pi = self.nce.main_repository.portage_interface

        self.assert_(isinstance(pi, RepositoryPortageInterface))

        path = os.path.join(os.getcwd(), "repository_TEST_dir/testrepo/profiles/testprofile")
        profile = pi.find_profile(path)

        self.assert_(isinstance(profile, RepositoryPortageInterfaceProfilesDescLine))
        self.assertEquals(profile.path, path)
        self.assertEquals(profile.arch, "x86")
        self.assertEquals(profile.status, "stable")


        self.assertEquals(pi.find_profile("broken"), None)

        profile = iter(pi.profiles).next()
        pi.profile = profile

        self.assertEquals(pi.profile_variable("ARCH"), "test")

if __name__ == "__main__":
    unittest.main()
