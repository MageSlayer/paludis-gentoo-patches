#!/usr/bin/env python
# vim: set fileencoding=utf-8 sw=4 sts=4 et :

#
# Copyright (c) 2008 Ciaran McCreesh
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

from paludis import *
import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_Generator(unittest.TestCase):
    def test_01_get(self):
        pass

    def test_02_str(self):
        env = TestEnvironment()
        spec = parse_user_package_dep_spec("foo/bar", env, [])

        self.assertEquals(str(Generator.All()), "all packages")
        self.assertEquals(
                str(Generator.Matches(spec, MatchPackageOptions())),
                "packages matching foo/bar")
        self.assertEquals(
                str(Generator.Matches(
                        spec,
                        [MatchPackageOption.IGNORE_ADDITIONAL_REQUIREMENTS])),
                "packages matching foo/bar (ignoring additional requirements)")


if __name__ == "__main__":
    unittest.main()

