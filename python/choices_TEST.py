#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2008, 2011 Ciaran McCreesh
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

repo_path = os.path.join(os.getcwd(), "choices_TEST_dir/testrepo")
ph = os.path.join(os.getcwd(), "choices_TEST_dir/home")
os.environ["PALUDIS_HOME"] = ph

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING


class TestCase_01_Choices(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")
        self.pid = next(
            iter(self.e.fetch_repository("testrepo").package_ids("foo/bar", []))
        )
        self.choices = self.pid.find_metadata("PALUDIS_CHOICES").parse_value()

    def test_01_choices(self):
        self.assertTrue(isinstance(self.choices, Choices), self.choices.__class__)

    def test_02_find_by_name_with_prefix(self):
        self.assertTrue(self.choices.find_by_name_with_prefix("testflag"))
        self.assertTrue(
            isinstance(self.choices.find_by_name_with_prefix("testflag"), ChoiceValue)
        )
        self.assertTrue(not self.choices.find_by_name_with_prefix("monkey"))

    def test_03_has_matching_contains_every_value_prefix(self):
        self.assertTrue(
            self.choices.has_matching_contains_every_value_prefix("linguas_en")
        )
        self.assertTrue(
            not self.choices.has_matching_contains_every_value_prefix("foo")
        )

    def test_04_iter(self):
        self.assertTrue(isinstance(next(iter(self.choices)), Choice))
        found = False
        for f in self.choices:
            if f.raw_name == "USE":
                found = True
        self.assertTrue(found)


class TestCase_02_Choice(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")
        self.pid = next(
            iter(self.e.fetch_repository("testrepo").package_ids("foo/bar", []))
        )
        self.choices = self.pid.find_metadata("PALUDIS_CHOICES").parse_value()
        self.use = None
        self.linguas = None
        for f in self.choices:
            if f.raw_name == "USE":
                self.use = f
            elif f.raw_name == "LINGUAS":
                self.linguas = f

    def test_01_use(self):
        self.assertTrue(self.use)
        self.assertEqual(self.use.raw_name, "USE")
        self.assertEqual(self.use.human_name, "USE")
        self.assertEqual(self.use.prefix, "")
        self.assertEqual(self.use.contains_every_value, False)
        self.assertEqual(self.use.hidden, False)
        self.assertEqual(self.use.show_with_no_prefix, True)
        self.assertEqual(self.use.consider_added_or_changed, True)

    def test_02_linguas(self):
        self.assertTrue(self.linguas)
        self.assertEqual(self.linguas.raw_name, "LINGUAS")
        self.assertEqual(self.linguas.human_name, "linguas")
        self.assertEqual(self.linguas.prefix, "linguas")
        self.assertEqual(self.linguas.contains_every_value, True)
        self.assertEqual(self.linguas.hidden, False)
        self.assertEqual(self.linguas.show_with_no_prefix, False)
        self.assertEqual(self.linguas.consider_added_or_changed, True)

    def test_03_use_iter(self):
        self.assertTrue(isinstance(next(iter(self.use)), ChoiceValue))
        found = False
        for f in self.use:
            if f.name_with_prefix == "testflag":
                found = True
        self.assertTrue(found)

    def test_04_linguas_iter(self):
        self.assertTrue(isinstance(next(iter(self.linguas)), ChoiceValue))
        found = False
        for f in self.linguas:
            if f.name_with_prefix == "linguas_en":
                found = True
        self.assertTrue(found)


class TestCase_03_ChoiceValue(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")
        self.pid = next(
            iter(self.e.fetch_repository("testrepo").package_ids("foo/bar", []))
        )
        self.choices = self.pid.find_metadata("PALUDIS_CHOICES").parse_value()
        self.use_testflag = self.choices.find_by_name_with_prefix("testflag")
        self.linguas_en = self.choices.find_by_name_with_prefix("linguas_en")

    def test_01_use_testflag(self):
        self.assertTrue(self.use_testflag)
        self.assertEqual(self.use_testflag.unprefixed_name, "testflag")
        self.assertEqual(self.use_testflag.name_with_prefix, "testflag")
        self.assertEqual(self.use_testflag.enabled, False)
        self.assertEqual(self.use_testflag.enabled_by_default, False)
        self.assertEqual(self.use_testflag.locked, False)
        self.assertEqual(self.use_testflag.description, "the test flag")
        self.assertEqual(self.use_testflag.origin, ChoiceOrigin.EXPLICIT)

    def test_02_linguas_en(self):
        self.assertTrue(self.linguas_en)
        self.assertEqual(self.linguas_en.unprefixed_name, "en")
        self.assertEqual(self.linguas_en.name_with_prefix, "linguas_en")
        self.assertEqual(self.linguas_en.enabled, False)
        self.assertEqual(self.linguas_en.enabled_by_default, False)
        self.assertEqual(self.linguas_en.locked, False)
        self.assertEqual(self.linguas_en.description, "English")
        self.assertEqual(self.linguas_en.origin, ChoiceOrigin.EXPLICIT)


if __name__ == "__main__":
    unittest.main()
