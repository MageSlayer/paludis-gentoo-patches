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

class TestCase_EAPI(unittest.TestCase):
    def test_01_instance(self):
        self.assert_(isinstance(EAPIData.instance, EAPIData))

    def test_02_create_error(self):
        self.assertRaises(Exception, SupportedEAPI)
        self.assertRaises(Exception, EAPI)

    def test_03_unknown_eapi(self):
        eapi = EAPIData.instance.unknown_eapi()

        self.assertEquals(eapi.name, "UNKNOWN")
        self.assertEquals(eapi.supported, None)

    def test_04_eapi(self):
        eapi = EAPIData.instance.eapi_from_string("foo")

        self.assertEquals(eapi.name, "foo")
        self.assertEquals(eapi.supported, None)

    def test_05_supported_eapi(self):
        eapi = EAPIData.instance.eapi_from_string("paludis-1")

        self.assertEquals(eapi.name, "paludis-1")
        self.assert_(isinstance(eapi.supported.package_dep_spec_parse_mode, PackageDepSpecParseMode))
        self.assert_(isinstance(eapi.supported.strict_package_dep_spec_parse_mode, PackageDepSpecParseMode))
        self.assert_(isinstance(eapi.supported.iuse_flag_parse_mode, IUseFlagParseMode))
        self.assert_(isinstance(eapi.supported.strict_iuse_flag_parse_mode, IUseFlagParseMode))
        self.assert_(isinstance(eapi.supported.breaks_portage, bool))
        self.assert_(isinstance(eapi.supported.has_pretend_phase, bool))

if __name__ == "__main__":
    unittest.main()
