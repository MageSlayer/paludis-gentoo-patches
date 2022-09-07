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


class TestCase_VersionRequirements(unittest.TestCase):
    def test_01_init(self):
        VersionRequirement("<", VersionSpec("0"))
        VersionRequirement(VersionOperatorValue.LESS, VersionSpec("0"))

    def test_02_data_members(self):
        v1 = VersionRequirement("<", VersionSpec("0"))
        v1.version_operator = ">"
        v1.version_spec = VersionSpec("1")

        self.assertEquals(str(v1.version_operator), ">")
        self.assertEquals(str(v1.version_spec), "1")


if __name__ == "__main__":
    unittest.main()
