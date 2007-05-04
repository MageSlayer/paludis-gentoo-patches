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

repo = os.path.join(os.getcwd(), "version_metadata_TEST_dir/testrepo")
irepo = os.path.join(os.getcwd(), "version_metadata_TEST_dir/installed")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING;

class TestCase_VersionMetadata(unittest.TestCase):
    e = NoConfigEnvironment(repo)
    evdb = NoConfigEnvironment(irepo)

    def vmd(self, version):
        return self.e.package_database.fetch_repository("testrepo").version_metadata("foo/bar", version)

    def vmd_vdb(self, package, version):
        return self.evdb.package_database.fetch_repository("installed").version_metadata(package, version)

    def test_01_get(self):
        self.vmd("1.0")

    def test_02_create_error(self):
        self.assertRaises(Exception, VersionMetadata)

    def test_03_data_members(self):
        vmd = self.vmd("1.0")

        self.assertEquals(str(vmd.slot), "0")
        self.assertEquals(vmd.homepage, "http://paludis.pioto.org/")
        self.assertEquals(vmd.description, "Test package")
        self.assertEquals(vmd.eapi, "0")

    def test_04_ebuild_interface(self):
        ei = self.vmd("1.0").ebuild_interface
        ei2 = self.vmd_vdb("cat-one/pkg-one", "1").ebuild_interface

        self.assert_(isinstance(ei, VersionMetadataEbuildInterface))
        self.assert_(isinstance(ei2, VersionMetadataEbuildInterface))

        self.assertEquals(ei.provide_string, "")
        self.assertEquals(ei.src_uri_string, "http://example.com/bar-1.0.tar.bz2")
        self.assertEquals(ei.restrict_string, "monkey")
        self.assertEquals(ei.keywords_string, "test ")
        self.assertEquals(ei.eclass_keywords, "")
        self.assertEquals(ei.iuse, " ")
        self.assertEquals(ei.inherited, "")

    def test_05_ebin_interface_TODO(self):
        pass

    def test_06_cran_interface_TODO(self):
        pass

    def test_07_deps_interface(self):
        di = self.vmd("1.0").deps_interface
        di2 = self.vmd_vdb("cat-one/pkg-one", "1").deps_interface

        self.assert_(isinstance(di, VersionMetadataDepsInterface))
        self.assert_(isinstance(di2, VersionMetadataDepsInterface))

        self.assertEquals(di.build_depend_string.strip(), "foo/bar")
        self.assertEquals(di.run_depend_string.strip(), "")
        self.assertEquals(di.post_depend_string.strip(), "")
        self.assertEquals(di.suggested_depend_string, "")

        self.assert_(isinstance(di.build_depend, AllDepSpec))
        self.assert_(isinstance(di.run_depend, AllDepSpec))
        self.assert_(isinstance(di.suggested_depend, AllDepSpec))
        self.assert_(isinstance(di.post_depend, AllDepSpec))

    def test_08_origins_interface(self):
        oi = self.vmd_vdb("cat-one/pkg-one", "1").origins_interface

        self.assert_(isinstance(oi, VersionMetadataOriginsInterface))

        self.assertEquals(str(oi.source), "cat-one/pkg-one-1::origin_test")
        self.assertEquals(oi.binary, None)

    def test_09_virutal_interface_TODO(self):
        pass

    def test_10_license_interface(self):
        li = self.vmd("1.0").license_interface
        li2 = self.vmd_vdb("cat-one/pkg-one", "1").license_interface

        self.assert_(isinstance(li, VersionMetadataLicenseInterface))
        self.assert_(isinstance(li2, VersionMetadataLicenseInterface))

        self.assertEquals(li.license_string, "GPL-2")
        self.assertEquals(li2.license_string, "")

if __name__ == "__main__":
    unittest.main()
