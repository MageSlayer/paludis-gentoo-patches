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

os.environ["PALUDIS_HOME"] = os.path.join(os.getcwd(), "mask_TEST_dir/home")

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING


class TestCase_01_Masks(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")

    def test_01_user_mask(self):
        q = Selection.RequireExactlyOne(
            Generator.Matches(
                parse_user_package_dep_spec("=masked/user-1.0", self.e, []), []
            )
        )
        pid = next(iter(self.e[q]))
        m = next(iter(pid.masks))

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UserMask))

        self.assertEqual(m.key(), "U")
        self.assertEqual(m.description(), "user")

    def test_02_unaccepted_mask(self):
        q = Selection.RequireExactlyOne(
            Generator.Matches(
                parse_user_package_dep_spec("=masked/unaccepted-1.0", self.e, []), []
            )
        )
        pid = next(iter(self.e[q]))
        m = next(iter(pid.masks))

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnacceptedMask))

        self.assertEqual(m.key(), "K")
        self.assertEqual(m.description(), "keyword")
        self.assertEqual(m.unaccepted_key_name(), "KEYWORDS")

    def test_03_repository_mask(self):
        q = Selection.RequireExactlyOne(
            Generator.Matches(
                parse_user_package_dep_spec("=masked/repo-1.0", self.e, []), []
            )
        )
        pid = next(iter(self.e[q]))
        m = next(iter(pid.masks))

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, RepositoryMask))

        self.assertEqual(m.key(), "R")
        self.assertEqual(m.description(), "repository")

        package_mask_path = os.path.realpath(
            os.path.join(os.getcwd(), "mask_TEST_dir/testrepo/profiles/package.mask")
        )

    def test_04_unsupported_mask(self):
        q = Selection.RequireExactlyOne(
            Generator.Matches(
                parse_user_package_dep_spec("=masked/unsupported-1.0", self.e, []), []
            )
        )
        pid = next(iter(self.e[q]))
        m = next(iter(pid.masks))

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnsupportedMask))

        self.assertEqual(m.key(), "E")
        self.assertEqual(m.description(), "eapi")
        self.assertEqual(m.explanation(), "Unsupported EAPI 'unsupported'")


class TestCase_02_Masks_subclassing(unittest.TestCase):
    def test_01_user_mask(self):
        class TestUserMask(UserMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

        test_user_mask(TestUserMask())

    def test_02_unaccepted_mask(self):
        class TestUnacceptedMask(UnacceptedMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def unaccepted_key_name(self):
                return "monkey"

        test_unaccepted_mask(TestUnacceptedMask())

    def test_03_repository_mask(self):
        class TestRepositoryMask(RepositoryMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

        test_repository_mask(TestRepositoryMask())

    def test_04_unsupported_mask(self):
        class TestUnsupportedMask(UnsupportedMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def explanation(self):
                return "test"

        test_unsupported_mask(TestUnsupportedMask())


if __name__ == "__main__":
    unittest.main()
