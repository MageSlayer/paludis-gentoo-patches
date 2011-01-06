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

os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "dep_list_TEST_dir/home")
repo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "package_id_TEST_dir/installed")

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_DepListOptions(unittest.TestCase):
    def setUp(self):
        global dlo
        dlo = DepListOptions()

    def test_01_create(self):
        pass

    def test_02_data_members_types(self):
        self.assert_(isinstance(dlo.reinstall, DepListReinstallOption))
        self.assert_(isinstance(dlo.reinstall_scm, DepListReinstallScmOption))
        self.assert_(isinstance(dlo.target_type, DepListTargetType))
        self.assert_(isinstance(dlo.upgrade, DepListUpgradeOption))
        self.assert_(isinstance(dlo.downgrade, DepListDowngradeOption))
        self.assert_(isinstance(dlo.new_slots, DepListNewSlotsOption))
        self.assert_(isinstance(dlo.fall_back, DepListFallBackOption))
        self.assert_(isinstance(dlo.installed_deps_pre, DepListDepsOption))
        self.assert_(isinstance(dlo.installed_deps_runtime, DepListDepsOption))
        self.assert_(isinstance(dlo.installed_deps_post, DepListDepsOption))
        self.assert_(isinstance(dlo.uninstalled_deps_pre, DepListDepsOption))
        self.assert_(isinstance(dlo.uninstalled_deps_runtime, DepListDepsOption))
        self.assert_(isinstance(dlo.uninstalled_deps_post, DepListDepsOption))
        self.assert_(isinstance(dlo.uninstalled_deps_suggested, DepListDepsOption))
        self.assert_(isinstance(dlo.suggested, DepListSuggestedOption))
        self.assert_(isinstance(dlo.circular, DepListCircularOption))
        self.assert_(isinstance(dlo.use, DepListUseOption))
        self.assert_(isinstance(dlo.blocks, DepListBlocksOption))
        self.assert_(isinstance(dlo.dependency_tags, bool))

    def test_03_data_members_set(self):
        dlo.reinstall = DepListReinstallOption.values[0]
        self.assertEquals(dlo.reinstall, DepListReinstallOption.values[0])

        dlo.reinstall_scm = DepListReinstallScmOption.values[0]
        self.assertEquals(dlo.reinstall_scm, DepListReinstallScmOption.values[0])

        dlo.target_type = DepListTargetType.values[0]
        self.assertEquals(dlo.target_type, DepListTargetType.values[0])

        dlo.upgrade = DepListUpgradeOption.values[0]
        self.assertEquals(dlo.upgrade, DepListUpgradeOption.values[0])

        dlo.downgrade = DepListDowngradeOption.values[0]
        self.assertEquals(dlo.downgrade, DepListDowngradeOption.values[0])

        dlo.new_slots = DepListNewSlotsOption.values[0]
        self.assertEquals(dlo.new_slots, DepListNewSlotsOption.values[0])

        dlo.fall_back = DepListFallBackOption.values[0]
        self.assertEquals(dlo.fall_back, DepListFallBackOption.values[0])

        dlo.installed_deps_pre = DepListDepsOption.values[0]
        self.assertEquals(dlo.installed_deps_pre, DepListDepsOption.values[0])

        dlo.installed_deps_runtime = DepListDepsOption.values[0]
        self.assertEquals(dlo.installed_deps_runtime, DepListDepsOption.values[0])

        dlo.installed_deps_post = DepListDepsOption.values[0]
        self.assertEquals(dlo.installed_deps_post, DepListDepsOption.values[0])

        dlo.uninstalled_deps_pre = DepListDepsOption.values[0]
        self.assertEquals(dlo.uninstalled_deps_pre, DepListDepsOption.values[0])

        dlo.uninstalled_deps_runtime = DepListDepsOption.values[0]
        self.assertEquals(dlo.uninstalled_deps_runtime, DepListDepsOption.values[0])

        dlo.uninstalled_deps_post = DepListDepsOption.values[0]
        self.assertEquals(dlo.uninstalled_deps_post, DepListDepsOption.values[0])

        dlo.uninstalled_deps_suggested = DepListDepsOption.values[0]
        self.assertEquals(dlo.uninstalled_deps_suggested, DepListDepsOption.values[0])

        dlo.suggested = DepListSuggestedOption.values[0]
        self.assertEquals(dlo.suggested, DepListSuggestedOption.values[0])

        dlo.circular = DepListCircularOption.values[0]
        self.assertEquals(dlo.circular, DepListCircularOption.values[0])

        dlo.use = DepListUseOption.values[0]
        self.assertEquals(dlo.use, DepListUseOption.values[0])

        dlo.blocks = DepListBlocksOption.values[0]
        self.assertEquals(dlo.blocks, DepListBlocksOption.values[0])

        dlo.dependency_tags = False
        self.assertEquals(dlo.dependency_tags, False)

class TestCase_02_DepList(unittest.TestCase):
    def setUp(self):
        global env, dl, dd, pds, cds
        env = EnvironmentFactory.instance.create("")
        dl = DepList(env, DepListOptions())
        dd = env.default_destinations
        pds = parse_user_package_dep_spec("foo/bar", env, [])
        cds = env.set("bar")

    def test_01_create(self):
        pass

    def test_02_options(self):
        self.assert_(isinstance(dl.options, DepListOptions))

    def test_04_clear(self):
        dl.add(pds, dd)
        dl.clear()

    def test_05_add(self):
        dl.add(pds, dd)
        dl.clear()
        dl.add(cds, dd)

    def test_06_already_installed(self):
        self.assert_(not dl.already_installed(pds, dd))
        dl.add(cds, dd)
        self.assert_(dl.already_installed(pds, dd))

    def test_07_entries(self):
        dl.add(pds, dd)

        entries = list(dl)
        self.assert_(isinstance(entries[0], DepListEntry))


class TestCase_03_DepListEntry(unittest.TestCase):
    def setUp(self):
        global env, dl, dd, pds, cds
        env = EnvironmentFactory.instance.create("")
        dl = DepList(env, DepListOptions())
        dd = env.default_destinations

    def test_01_no_create(self):
        self.assertRaises(Exception, DepListEntry)

    def test_02_data_members(self):
        dl.add(pds, dd)

        dle = list(dl)[0]

        self.assertEquals(dle.kind, DepListEntryKind.PACKAGE)
        self.assertEquals(dle.package_id.name, "foo/bar")
        self.assertEquals(list(dle.tags), [])
        self.assertEquals(str(dle.destination.name), "installed")
        self.assertEquals(dle.state, DepListEntryState.HAS_ALL_DEPS)


if __name__ == "__main__":
    unittest.main()

