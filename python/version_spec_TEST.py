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


class TestCase_VersionSpec(unittest.TestCase):
    def test_01_init(self):
        VersionSpec("0")

    def test_02_exceptions(self):
        self.assertRaises(BadVersionSpecError, VersionSpec, "1.0-r1-")

    def test_03_compare(self):
        v0 = VersionSpec("0")
        v1 = VersionSpec("0.1")
        v2 = VersionSpec("1.0_beta3")
        v3 = VersionSpec("1.0")

        self.assertTrue(v0 < v1)
        self.assertTrue(v1 < v2)
        self.assertTrue(v2 < v3)

        self.assertTrue(v0 >= v0)
        self.assertTrue(v1 >= v1)
        self.assertTrue(v3 >= v2)

    def test_04_str(self):
        self.assertEqual("0.1_beta2-r3", str(VersionSpec("0.1_beta2-r3")))
        self.assertEqual("1", str(VersionSpec("0.1_beta2-r3").bump()))

    def test_05_remove_revision(self):
        self.assertEqual(VersionSpec("0.1"), VersionSpec("0.1-r1").remove_revision())

    def test_06_revision_only(self):
        self.assertEqual("r3", VersionSpec("0.1r_alpha1-r3").revision_only())

    def test_07_bump(self):
        self.assertEqual(VersionSpec("2"), VersionSpec("1").bump())
        self.assertEqual(VersionSpec("2"), VersionSpec("1.2").bump())
        self.assertEqual(VersionSpec("1.3"), VersionSpec("1.2.3").bump())
        self.assertEqual(VersionSpec("1.3"), VersionSpec("1.2.3_beta1-r4").bump())
        self.assertEqual(VersionSpec("scm"), VersionSpec("scm").bump())

    def test_08_is_scm(self):
        self.assertTrue(VersionSpec("scm").is_scm)
        self.assertTrue(VersionSpec("9999").is_scm)
        self.assertTrue(not VersionSpec("1").is_scm)

    def test_09_has_scm_part(self):
        self.assertTrue(VersionSpec("1-scm").has_scm_part)
        self.assertTrue(not VersionSpec("1").has_scm_part)

    def test_09_has_scm_part(self):
        self.assertTrue(VersionSpec("1-try").has_try_part)
        self.assertTrue(not VersionSpec("1").has_try_part)


if __name__ == "__main__":
    unittest.main()
