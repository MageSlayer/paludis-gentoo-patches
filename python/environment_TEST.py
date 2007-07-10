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

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_Environments(unittest.TestCase):
    def setUp(self):
        global e, nce
        e = EnvironmentMaker.instance.make_from_spec("")
        nce = NoConfigEnvironment(repo)

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
        pid = iter(e.package_database.query(Query.Matches(
            PackageDepSpec("=foo/bar-1.0", PackageDepSpecParseMode.PERMISSIVE)),
            QueryOrder.REQUIRE_EXACTLY_ONE)).next()

        self.assert_(e.query_use("enabled", pid))
        self.assert_(not e.query_use("not_enabled", pid))
        self.assert_(e.query_use("sometimes_enabled", pid))

        self.assert_(not nce.query_use("foo", pid))

    def test_05_mask_reasons(self):
        pid = iter(nce.package_database.query(Query.Matches(
            PackageDepSpec("=foo/bar-1.0", PackageDepSpecParseMode.PERMISSIVE)),
            QueryOrder.REQUIRE_EXACTLY_ONE)).next()

        nce.mask_reasons(pid)

    def test_06_package_database(self):
        self.assert_(isinstance(e.package_database, PackageDatabase))
        self.assert_(isinstance(nce.package_database, PackageDatabase))

    def test_07_sets(self):
        self.assert_(isinstance(e.set("everything"), AllDepSpec))
        self.assert_(isinstance(nce.set("everything"), AllDepSpec))

        self.assert_(isinstance(e.set_names, SetNameIterable))
        self.assert_(isinstance(nce.set_names, SetNameIterable))

    def test_08_repositories(self):
        nce2 = NoConfigEnvironment(repo, master_repository_dir=slaverepo)

        self.assert_(isinstance(nce.main_repository, Repository))
        self.assertEquals(nce.master_repository, None)
        self.assert_(isinstance(nce2.main_repository, Repository))
        self.assert_(isinstance(nce2.master_repository, Repository))

    def test_09_root(self):
        self.assert_(isinstance(e.root, str))
        self.assert_(isinstance(nce.root, str))

    def test_10_default_destinations(self):
        self.assert_(isinstance(e.default_destinations, DestinationsIterable))
        self.assert_(isinstance(nce.default_destinations, DestinationsIterable))

    def test_11_set_accept_unstable(self):
        nce.accept_unstable = True
        self.assertRaises(AttributeError, lambda: nce.accept_unstable)

    def test_12_config_dir(self):
        self.assert_(isinstance(e.config_dir, str))

class TestCase_02_AdaptedEnvironment(unittest.TestCase):
    def test_01_create(self):
        env = AdaptedEnvironment(EnvironmentMaker.instance.make_from_spec(""))

    def test_02_adapt_use(self):
        env = AdaptedEnvironment(EnvironmentMaker.instance.make_from_spec(""))
        pid = iter(env.package_database.query(Query.Matches(
            PackageDepSpec("=foo/bar-1.0", PackageDepSpecParseMode.PERMISSIVE)),
            QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        pds = PackageDepSpec("foo/bar", PackageDepSpecParseMode.PERMISSIVE)

        self.assert_(env.query_use("enabled", pid))
        self.assert_(not env.query_use("not_enabled", pid))
        self.assert_(env.query_use("sometimes_enabled", pid))

        env.adapt_use(pds, "enabled", UseFlagState.DISABLED)
        self.assert_(not env.query_use("enabled", pid))

        env.adapt_use(pds, "not_enabled", UseFlagState.ENABLED)
        self.assert_(env.query_use("not_enabled", pid))

        env.adapt_use(pds, "sometimes_enabled", UseFlagState.ENABLED)
        self.assert_(env.query_use("sometimes_enabled", pid))

    def test_03_clear_adaptions(self):
        env = AdaptedEnvironment(EnvironmentMaker.instance.make_from_spec(""))
        pid = iter(env.package_database.query(Query.Matches(
            PackageDepSpec("=foo/bar-1.0", PackageDepSpecParseMode.PERMISSIVE)),
            QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        pds = PackageDepSpec("foo/bar", PackageDepSpecParseMode.PERMISSIVE)

        self.assert_(env.query_use("enabled", pid))

        env.adapt_use(pds, "enabled", UseFlagState.DISABLED)
        self.assert_(not env.query_use("enabled", pid))

        env.clear_adaptions()
        self.assert_(env.query_use("enabled", pid))

class TestCase_03_TestEnvironment(unittest.TestCase):
    def test_01_create(self):
        env = TestEnvironment()

if __name__ == "__main__":
    unittest.main()
