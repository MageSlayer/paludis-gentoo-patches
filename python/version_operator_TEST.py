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


class TestCase_VersionOperator(unittest.TestCase):
    def test_01_init(self):
        VersionOperator("<")
        VersionOperator(VersionOperatorValue.LESS)

    def test_02_exceptions(self):
        self.assertRaises(BadVersionOperatorError, VersionOperator, "<>")

    def test_03_str(self):
        self.assertEqual(">", str(VersionOperator(">")))
        self.assertEqual("<", str(VersionOperator(VersionOperatorValue.LESS)))

    def test_04_compare(self):
        self.assert_(
            VersionOperator("<").compare(VersionSpec("1.0"), VersionSpec("2.0"))
        )
        self.assert_(
            VersionOperator(">").compare(VersionSpec("3.0"), VersionSpec("2.0"))
        )
        self.assert_(
            VersionOperator(VersionOperatorValue.EQUAL_STAR).compare(
                VersionSpec("2.0.1-r1"), VersionSpec("2.0")
            )
        )


if __name__ == "__main__":
    unittest.main()
