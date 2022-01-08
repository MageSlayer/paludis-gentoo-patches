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

ph = os.path.join(os.getcwd(), "environment_TEST_dir/home")
repo = os.path.join(os.getcwd(), "environment_TEST_dir/testrepo")
slaverepo = os.path.join(os.getcwd(), "environment_TEST_dir/slaverepo")
os.environ["PALUDIS_HOME"] = ph

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_Environments(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")

    def test_02_create_error(self):
        self.assertRaises(Exception, Environment)

    def test_07_sets(self):
        self.assert_(isinstance(self.e.set("everything"), AllDepSpec))
        self.assert_(isinstance(self.e.set_names, SetNameIterable))

    def test_12_config_dir(self):
        self.assert_(isinstance(self.e.config_dir, str))

    def test_23_fetch_unique_qpn(self):
        self.assertEqual(str(QualifiedPackageName("foo/bar")), str(self.e.fetch_unique_qualified_package_name("bar")))
        self.assertEqual(str(QualifiedPackageName("foo/bar")), str(self.e.fetch_unique_qualified_package_name("bar",
            Filter.SupportsInstallAction())))

    def test_24_exceptions(self):
        self.assertRaises(NoSuchPackageError, self.e.fetch_unique_qualified_package_name, "baz")
        self.assertRaises(NoSuchPackageError, self.e.fetch_unique_qualified_package_name, "foobarbaz")
        self.assertRaises(NoSuchPackageError, self.e.fetch_unique_qualified_package_name, "bar",
                Filter.SupportsUninstallAction())

    def test_25_reduced(self):
        self.assert_(self.e.reduced_username() != "")
        self.assert_(self.e.reduced_uid() >= 0)
        self.assert_(self.e.reduced_gid() >= 0)

class TestCase_03_TestEnvironment(unittest.TestCase):
    def test_01_create(self):
        e = TestEnvironment()

class TestCase_04_Environment_subclassingd(unittest.TestCase):
    class SubEnv(EnvironmentImplementation):
        def __init__(self):
            EnvironmentImplementation.__init__(self)

        def accept_license(self, l, pid):
            return False

        def accept_keywords(self, kns, pid):
            return False

        def mask_for_user(self, pid, b):
            return UserMask()

        def unmasked_by_user(self, pid, reason):
            return False

        def bashrc_files(self):
            return ["/path"]

        def syncers_dirs(self):
            return ["/path"]

        def fetchers_dirs(self):
            return ["/path"]

        def hook_dirs(self):
            return ["/path"]

        def reduced_uid(self):
            return 0

        def reduced_gid(self):
            return 0

        def mirrors(self, mirror):
            return ["mirror"]

        def set_names(self):
            return ["set"]

        def set(self, set):
            return AllDepSpec()

        def distribution(self):
            return EnvironmentImplementation.distribution(self)

        def need_keys_added(self):
            return

    def test_01_environment_implementation(self):
        test_env(self.SubEnv())

if __name__ == "__main__":
    unittest.main()
