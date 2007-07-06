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

repo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/installed")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_MetadataKeys(unittest.TestCase):
    def setUp(self):
        global e, ie, pid, ipid
        e = NoConfigEnvironment(repo_path, "/var/empty")
        ie = NoConfigEnvironment(irepo_path)
        pid = iter(e.package_database.fetch_repository("testrepo").package_ids("foo/bar")).next()
        ipid = iter(ie.package_database.fetch_repository("installed").package_ids("cat-one/pkg-one")).next()

    def test_01_contents(self):
        self.assertEquals(pid.find("CONTENTS"), None)
        self.assert_(isinstance(ipid.find("CONTENTS"), MetadataContentsKey))

    def test_02_installed_time(self):
        self.assertEquals(pid.find("INSTALLED_TIME"), None)
        self.assert_(isinstance(ipid.find("INSTALLED_TIME"), MetadataTimeKey))

    def test_03_repository(self):
        self.assertEquals(pid.find("REPOSITORY"), None)
        self.assert_(isinstance(ipid.find("REPOSITORY"), MetadataStringKey))

    def test_04_keywords(self):
        self.assert_(isinstance(pid.find("KEYWORDS"), MetadataKeywordNameIterableKey))
        self.assertEquals(ipid.find("KEYWORDS"), None)

    def test_05_use(self):
        self.assertEquals(pid.find("USE"), None)
        self.assert_(isinstance(ipid.find("USE"), MetadataUseFlagNameIterableKey))

    def test_06_iuse(self):
        self.assert_(isinstance(pid.find("IUSE"), MetadataIUseFlagIterableKey))
        self.assert_(isinstance(ipid.find("IUSE"), MetadataIUseFlagIterableKey))

    def test_07_inherited(self):
        self.assert_(isinstance(pid.find("INHERITED"), MetadataInheritedIterableKey))
        self.assert_(isinstance(ipid.find("INHERITED"), MetadataInheritedIterableKey))

    def test_08_depend(self):
        self.assert_(isinstance(pid.find("DEPEND"), MetadataDependencySpecTreeKey))
        self.assertEquals(ipid.find("DEPEND"), None)

if __name__ == "__main__":
    unittest.main()
