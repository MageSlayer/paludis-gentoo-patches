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

class TestCase_01_PortageDepParser(unittest.TestCase):
    def setUp(self):
        global eapi
        eapi = EAPIData.instance.eapi_from_string("0")

    def test_01_init_error(self):
        self.assertRaises(Exception, PortageDepParser)

    def test_02_parse_depend(self):
        spec = PortageDepParser.parse_depend("foo/boo", eapi)
        self.assert_(isinstance(spec, AllDepSpec))
        self.assert_(isinstance(iter(spec).next(), PackageDepSpec))
        self.assertEquals(len(list(spec)), 1)

    def test_03_parse_provide(self):
        spec = PortageDepParser.parse_provide("foo/boo", eapi)
        self.assert_(isinstance(spec, AllDepSpec))
        self.assert_(isinstance(iter(spec).next(), PackageDepSpec))
        self.assertEquals(len(list(spec)), 1)

    def test_04_parse_restrict(self):
        spec = PortageDepParser.parse_restrict("foo/boo", eapi)
        self.assert_(isinstance(spec, AllDepSpec))
        self.assert_(isinstance(iter(spec).next(), PlainTextDepSpec))
        self.assertEquals(len(list(spec)), 1)

    def test_05_parse_uri(self):
        spec = PortageDepParser.parse_uri("http://foo/boo", eapi)
        self.assert_(isinstance(spec, AllDepSpec))
        self.assert_(isinstance(iter(spec).next(), URIDepSpec))
        self.assertEquals(len(list(spec)), 1)


    def test_06_parse_license(self):
        spec = PortageDepParser.parse_license("FOO_LICENSE", eapi)
        self.assert_(isinstance(spec, AllDepSpec))
        self.assert_(isinstance(iter(spec).next(), PlainTextDepSpec))
        self.assertEquals(len(list(spec)), 1)

    def test_07_exceptions(self):
        self.assertRaises(DepStringLexError, PortageDepParser.parse_depend, "(foo/boo )", eapi)
        self.assertRaises(DepStringNestingError, PortageDepParser.parse_license, "|| ( foo/boo ", eapi)
        self.assertRaises(DepStringParseError, PortageDepParser.parse_provide, "|| ( foo/boo )", eapi)

if __name__ == "__main__":
    unittest.main()
