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
        global e, ie, pid, ipid
        e = NoConfigEnvironment(repo_path, "/var/empty")
        ie = NoConfigEnvironment(irepo_path)
        pid = iter(e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
        ipid = iter(ie.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one")).next()

    def test_01_get(self):
        pass

    def test_02_name(self):
        self.assertEquals(pid.name, QualifiedPackageName("foo/bar"))
        self.assertEquals(ipid.name, QualifiedPackageName("cat-one/pkg-one"))

    def test_03_version(self):
        self.assertEquals(pid.version, VersionSpec("1.0"))
        self.assertEquals(ipid.version, VersionSpec("1"))

    def test_04_slot(self):
        self.assertEquals(str(ipid.slot), "test_slot")
        self.assertEquals(str(pid.slot), "0")

    def test_05_repository(self):
        self.assertEquals(str(pid.repository.name), "testrepo")
        self.assertEquals(str(ipid.repository.name), "installed")

    def test_07_canonical_form(self):
        self.assertEquals(pid.canonical_form(PackageIDCanonicalForm.FULL), "foo/bar-1.0::testrepo")
        self.assertEquals(pid.canonical_form(PackageIDCanonicalForm.VERSION), "1.0")
        self.assertEquals(pid.canonical_form(PackageIDCanonicalForm.NO_VERSION), "foo/bar::testrepo")

        self.assertEquals(ipid.canonical_form(PackageIDCanonicalForm.FULL), "cat-one/pkg-one-1::installed")
        self.assertEquals(ipid.canonical_form(PackageIDCanonicalForm.VERSION), "1")
        self.assertEquals(ipid.canonical_form(PackageIDCanonicalForm.NO_VERSION), "cat-one/pkg-one::installed")

    def test_08_find_metadata(self):
        self.assert_(isinstance(pid.find_metadata("DEPEND"), MetadataDependencySpecTreeKey))

if __name__ == "__main__":
    unittest.main()
