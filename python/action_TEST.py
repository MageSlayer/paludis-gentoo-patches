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

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING


class TestCase_01_InstallActionOptions(unittest.TestCase):
    def setUp(self):
        global repo1, repo2
        env = TestEnvironment()
        repo1 = FakeRepository(env, "1")
        repo2 = FakeRepository(env, "2")

    def test_01_create(self):
        InstallActionOptions(repo1)

    def test_02_data_members(self):
        iao = InstallActionOptions(repo1)

        self.assertEquals(str(iao.destination.name), "1")

        iao.no_config_protect = False
        iao.destination = repo2

        self.assertEquals(iao.no_config_protect, False)
        self.assertEquals(str(iao.destination.name), "2")


class TestCase_02_FetchActionOptions(unittest.TestCase):
    def test_01_create(self):
        FetchActionOptions(True, True, True)

    def test_02_data_members(self):
        fao = FetchActionOptions(True, True, True)

        self.assertEquals(fao.exclude_unmirrorable, True)
        self.assertEquals(fao.safe_resume, True)

        fao.exclude_unmirrorable = False
        fao.safe_resume = False

        self.assertEquals(fao.exclude_unmirrorable, False)
        self.assertEquals(fao.safe_resume, False)


class TestCase_04_InstallAction(unittest.TestCase):
    def test_01_create(self):
        env = TestEnvironment()
        repo1 = FakeRepository(env, "1")
        iao = InstallActionOptions(repo1)
        InstallAction(iao)


class TestCase_05_FetchAction(unittest.TestCase):
    def test_01_create(self):
        FetchAction(FetchActionOptions(False, True, True))


class TestCase_06_UninstallAction(unittest.TestCase):
    def test_01_create(self):
        UninstallAction(UninstallActionOptions("monkey"))


class TestCase_08_PretendAction(unittest.TestCase):
    def test_01_create(self):
        env = TestEnvironment()
        repo1 = FakeRepository(env, "1")
        pao = PretendActionOptions(repo1)
        PretendAction(pao)


class TestCase_09_ConfigAction(unittest.TestCase):
    def test_01_create(self):
        ConfigAction(ConfigActionOptions())


class TestCase_10_InfoAction(unittest.TestCase):
    def test_01_create(self):
        InfoAction(InfoActionOptions())


class TestCase_10_SupportsActionTests(unittest.TestCase):
    def test_01_create(self):
        SupportsInstallActionTest()
        SupportsFetchActionTest()
        SupportsUninstallActionTest()
        SupportsPretendActionTest()
        SupportsConfigActionTest()
        SupportsInfoActionTest()


if __name__ == "__main__":
    unittest.main()
