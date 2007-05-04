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

from paludis import *
import unittest

class TestCase_01_PortageDepParser_Policy(unittest.TestCase):
    def test_01_init_error(self):
        self.assertRaises(Exception, PortageDepParser.Policy)

    def test_02_create_text(self):
        PortageDepParser.Policy.text_is_text_dep_spec(True)

    def test_03_create_package(self):
        PortageDepParser.Policy.text_is_package_dep_spec(True, PackageDepSpecParseMode.PERMISSIVE)

class TestCase_02_PortageDepParser(unittest.TestCase):
    def get_policies(self):
        self.text_true = PortageDepParser.Policy.text_is_text_dep_spec(True)
        self.text_false = PortageDepParser.Policy.text_is_text_dep_spec(False)
        self.package_true = PortageDepParser.Policy.text_is_package_dep_spec(True, PackageDepSpecParseMode.PERMISSIVE)
        self.package_false = PortageDepParser.Policy.text_is_package_dep_spec(False, PackageDepSpecParseMode.PERMISSIVE)

    def test_01_init_error(self):
        self.assertRaises(Exception, PortageDepParser)

    def test_02_parse(self):
        self.get_policies()

        spec1 = PortageDepParser.parse("foo/boo", self.text_true)
        self.assert_(isinstance(spec1, AllDepSpec))
        self.assert_(isinstance(iter(spec1).next(), PlainTextDepSpec))
        self.assertEquals(len(list(spec1)), 1)
        self.assertEquals(str(iter(spec1).next()), "foo/boo")

        spec2 = PortageDepParser.parse("foo/boo", self.text_false)
        self.assert_(isinstance(spec2, AllDepSpec))
        self.assert_(isinstance(iter(spec2).next(), PlainTextDepSpec))
        self.assertEquals(len(list(spec2)), 1)
        self.assertEquals(str(iter(spec2).next()), "foo/boo")

        spec3 = PortageDepParser.parse("foo/boo", self.package_true)
        self.assert_(isinstance(spec3, AllDepSpec))
        self.assert_(isinstance(iter(spec3).next(), PackageDepSpec))
        self.assertEquals(len(list(spec3)), 1)
        self.assertEquals(str(iter(spec3).next()), "foo/boo")

        spec4 = PortageDepParser.parse("foo/boo", self.package_false)
        self.assert_(isinstance(spec4, AllDepSpec))
        self.assert_(isinstance(iter(spec4).next(), PackageDepSpec))
        self.assertEquals(len(list(spec4)), 1)
        self.assertEquals(str(iter(spec4).next()), "foo/boo")

        PortageDepParser.parse("|| ( foo/boo )", self.text_true)
        PortageDepParser.parse("|| ( foo/boo )", self.package_true)

    def test_03_exceptions(self):
        self.get_policies()

        self.assertRaises(DepStringParseError, PortageDepParser.parse, "|| ( foo/boo )", self.text_false)
        self.assertRaises(DepStringParseError, PortageDepParser.parse, "|| ( foo/boo )", self.package_false)
        self.assertRaises(DepStringNestingError, PortageDepParser.parse, "|| ( foo/boo", self.package_true)

if __name__ == "__main__":
    unittest.main()
