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

repo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/installed")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_PackageID(unittest.TestCase):
    def setUp(self):
        self.e = NoConfigEnvironment(repo_path, "/var/empty")
        self.ie = NoConfigEnvironment(irepo_path)
        self.pid = iter(self.e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
        self.ipid = iter(self.ie.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one")).next()
        self.mpid = iter(self.e.package_database.fetch_repository("testrepo").package_ids("cat/masked")).next()

    def test_01_get(self):
        pass

    def test_02_name(self):
        self.assertEquals(self.pid.name, QualifiedPackageName("foo/bar"))
        self.assertEquals(self.ipid.name, QualifiedPackageName("cat-one/pkg-one"))

    def test_03_version(self):
        self.assertEquals(self.pid.version, VersionSpec("1.0"))
        self.assertEquals(self.ipid.version, VersionSpec("1"))

    def test_04_slot(self):
        self.assertEquals(str(self.ipid.slot), "test_slot")
        self.assertEquals(str(self.pid.slot), "0")

    def test_05_repository(self):
        self.assertEquals(str(self.pid.repository.name), "testrepo")
        self.assertEquals(str(self.ipid.repository.name), "installed")

    def test_07_canonical_form(self):
        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.FULL), "foo/bar-1.0::testrepo")
        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.VERSION), "1.0")
        self.assertEquals(self.pid.canonical_form(PackageIDCanonicalForm.NO_VERSION), "foo/bar::testrepo")

        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.FULL), "cat-one/pkg-one-1::installed")
        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.VERSION), "1")
        self.assertEquals(self.ipid.canonical_form(PackageIDCanonicalForm.NO_VERSION), "cat-one/pkg-one::installed")

    def test_08_find_metadata(self):
        self.assert_(isinstance(self.pid.find_metadata("DEPEND"), MetadataDependencySpecTreeKey))

    def test_09_perform_action(self):
        self.pid.perform_action(PretendAction())
        self.assertRaises(UnsupportedActionError, self.pid.perform_action, ConfigAction())
        self.assertRaises(UnsupportedActionError, self.ipid.perform_action, PretendAction())

    def test_10_supports_action(self):
        self.assert_(self.pid.supports_action(SupportsFetchActionTest()))
        self.assert_(self.pid.supports_action(SupportsInstallActionTest()))
        self.assert_(self.pid.supports_action(SupportsFetchActionTest()))
        self.assert_(not self.pid.supports_action(SupportsUninstallActionTest()))
        self.assert_(not self.pid.supports_action(SupportsInstalledActionTest()))
        self.assert_(self.pid.supports_action(SupportsPretendActionTest()))
        self.assert_(not self.pid.supports_action(SupportsConfigActionTest()))

        self.assert_(not self.ipid.supports_action(SupportsFetchActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsInstallActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsFetchActionTest()))
        self.assert_(self.ipid.supports_action(SupportsUninstallActionTest()))
        self.assert_(self.ipid.supports_action(SupportsInstalledActionTest()))
        self.assert_(not self.ipid.supports_action(SupportsPretendActionTest()))
        self.assert_(self.ipid.supports_action(SupportsConfigActionTest()))

    def test_11_masked(self):
        self.assert_(not self.pid.masked)
        self.assert_(self.mpid.masked)

    def test_12_masks(self):
        mask = iter(self.mpid.masks).next()
        self.assert_(isinstance(mask, UnacceptedMask))

if __name__ == "__main__":
    unittest.main()
