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


class TestCase_Names(unittest.TestCase):
    def test_1_create(self):
        self.names = {}
        self.names["cat-foo"] = CategoryNamePart("cat-foo")
        self.names["pkg"] = PackageNamePart("pkg")
        self.names["cat-foo/pkg"] = QualifiedPackageName(
            self.names["cat-foo"], self.names["pkg"]
        )
        self.names["cat-blah/pkg"] = QualifiedPackageName("cat-blah/pkg")
        self.names["3.3"] = SlotName("3.3")
        self.names["repo"] = RepositoryName("repo")
        self.names["keyword"] = KeywordName("keyword")
        self.names["set"] = SetName("set")

    def test_2_create_error(self):
        self.assertRaises(PackageNamePartError, PackageNamePart, ":bad")
        self.assertRaises(CategoryNamePartError, CategoryNamePart, ":bad")
        self.assertRaises(CategoryNamePartError, QualifiedPackageName, ":bad")
        self.assertRaises(SlotNameError, SlotName, ":bad")
        self.assertRaises(RepositoryNameError, RepositoryName, ":bad")
        self.assertRaises(KeywordNameError, KeywordName, ":bad")
        self.assertRaises(SetNameError, SetName, ":bad")
        self.assertRaises(Exception, PackageNamePartIterable)
        self.assertRaises(Exception, CategoryNamePartIterable)
        self.assertRaises(Exception, QualifiedPackageNameIterable)

    def test_3_str(self):
        self.test_1_create()
        for (k, v) in self.names.items():
            self.assertEqual(str(v), k)

    def test_4_operators(self):
        self.assert_(
            CategoryNamePart("cat-foo") + PackageNamePart("pkg")
            == QualifiedPackageName("cat-foo/pkg")
        )

    def test_5_data_members(self):
        qpn = QualifiedPackageName("cat/foo")
        self.assertEqual(str(qpn.category), "cat")
        self.assertEqual(str(qpn.package), "foo")


if __name__ == "__main__":
    unittest.main()
