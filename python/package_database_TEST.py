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

os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "package_database_TEST_dir/home")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_PackageDatabase(unittest.TestCase):
    def get_db(self):
        self.env = EnvironmentFactory.instance.create("")
        self.db = self.env.package_database

    def test_01_create_error(self):
        self.assertRaises(Exception, PackageDatabase)

    def test_03_fech_unique_qpn(self):
        self.get_db()
        self.assertEqual(str(QualifiedPackageName("foo/bar")), str(self.db.fetch_unique_qualified_package_name("bar")))
        self.assertEqual(str(QualifiedPackageName("foo/bar")), str(self.db.fetch_unique_qualified_package_name("bar",
            Filter.SupportsInstallAction())))

    def test_04_exceptions(self):
        self.get_db()
        self.assertRaises(AmbiguousPackageNameError, self.db.fetch_unique_qualified_package_name, "baz")
        self.assertRaises(NoSuchPackageError, self.db.fetch_unique_qualified_package_name, "foobarbaz")
        self.assertRaises(NoSuchPackageError, self.db.fetch_unique_qualified_package_name, "bar",
                Filter.SupportsUninstallAction())

    def test_6_repositories(self):
        if os.environ.get("PALUDIS_ENABLE_VIRTUALS_REPOSITORY") == "yes":
            self.get_db()
            self.assert_(self.db.more_important_than("testrepo", "virtuals"))
            self.assert_(not self.db.more_important_than("virtuals", "testrepo"))
            self.assertRaises(NoSuchRepositoryError, self.db.fetch_repository, "blah")

            self.assertEqual(len(list(self.db.repositories)), 3)

if __name__ == "__main__":
    unittest.main()

