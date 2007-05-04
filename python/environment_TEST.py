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

ph = os.path.join(os.getcwd(), "environment_TEST_dir/home")
repo = os.path.join(os.getcwd(), "environment_TEST_dir/testrepo")
slaverepo = os.path.join(os.getcwd(), "environment_TEST_dir/slaverepo")
os.environ["PALUDIS_HOME"] = ph

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING;

class TestCase_Environments(unittest.TestCase):
    def get_envs(self):
        self.e = EnvironmentMaker.instance.make_from_spec("")
        self.nce = NoConfigEnvironment(repo)

    def test_01_create(self):
        NoConfigEnvironment(repo)
        NoConfigEnvironment(repo, "/var/empty")
        NoConfigEnvironment(repo, "/var/empty", "/var/empty")
        NoConfigEnvironment(repo, master_repository_dir="/var/empty")
        NoConfigEnvironment(repo, write_cache_dir="/var/empty")

    def test_02_create_error(self):
        self.assertRaises(Exception, Environment)
        self.assertRaises(Exception, NoConfigEnvironment)

    def test_03_subclass(self):
        self.assert_(isinstance(NoConfigEnvironment(repo), Environment))

    def test_04_query_use(self):
        self.get_envs()

        pde = PackageDatabaseEntry("x/x", "1.0", "testrepo")

        self.assert_(self.e.query_use("enabled", pde))
        self.assert_(not self.e.query_use("not_enabled", pde))
        self.assert_(not self.e.query_use("sometimes_enabled", pde))

        pde = PackageDatabaseEntry("foo/bar", "1.0", "testrepo")

        self.assert_(self.e.query_use("enabled", pde))
        self.assert_(not self.e.query_use("not_enabled", pde))
        self.assert_(self.e.query_use("sometimes_enabled", pde))

        self.assert_(not self.nce.query_use("foo", pde))

    def test_05_mask_reasons(self):
        self.get_envs()
        pde = PackageDatabaseEntry("foo/bar", "1.0", "testrepo")

        self.nce.mask_reasons(pde)

    def test_06_package_database(self):
        self.get_envs()

        self.assert_(isinstance(self.e.package_database, PackageDatabase))
        self.assert_(isinstance(self.nce.package_database, PackageDatabase))

    def test_07_sets(self):
        self.get_envs()

        self.assert_(isinstance(self.e.set("everything"), AllDepSpec))
        self.assert_(isinstance(self.nce.set("everything"), AllDepSpec))

        self.assert_(isinstance(self.e.set_names(), SetNameCollection))
        self.assert_(isinstance(self.nce.set_names(), SetNameCollection))

    def test_08_repositories(self):
        self.get_envs()
        self.nce2 = NoConfigEnvironment(repo, master_repository_dir=slaverepo)

        self.assert_(isinstance(self.nce.main_repository, Repository))
        self.assertEquals(self.nce.master_repository, None)
        self.assert_(isinstance(self.nce2.main_repository, Repository))
        self.assert_(isinstance(self.nce2.master_repository, Repository))

    def test_09_root(self):
        self.get_envs()

        self.assert_(isinstance(self.e.root(), str))
        self.assert_(isinstance(self.nce.root(), str))

    def test_10_default_destinations(self):
        self.get_envs()

        self.assert_(isinstance(self.e.default_destinations(), DestinationsCollection))
        self.assert_(isinstance(self.nce.default_destinations(), DestinationsCollection))

    def test_11_set_accept_unstable(self):
        self.get_envs()

        self.nce.accept_unstable = True
        self.assertRaises(AttributeError, lambda: self.nce.accept_unstable)

    def test_12_config_dir(self):
        self.get_envs()

        self.assert_(isinstance(self.e.config_dir, str))

if __name__ == "__main__":
    unittest.main()
