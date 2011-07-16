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

class TestCase_1_DepSpecs(unittest.TestCase):
    def get_depspecs(self):
        self.env = TestEnvironment()
        self.ptds = PlainTextDepSpec("foo")
        self.pds = parse_user_package_dep_spec(">=foo/bar-1:100::testrepo", self.env, [])
        self.pds2 = parse_user_package_dep_spec("*/*::testrepo", self.env,
                UserPackageDepSpecOptions() + UserPackageDepSpecOption.ALLOW_WILDCARDS)
        self.pds3 = parse_user_package_dep_spec("*/*::testrepo", self.env,
                [UserPackageDepSpecOption.ALLOW_WILDCARDS])
        self.pds4 = parse_user_package_dep_spec("cat/pkg::testrepo", self.env, [])
        self.bds = BlockDepSpec("!>=foo/bar-1:100::testrepo", self.pds)
        self.nds = NamedSetDepSpec("system")

    def test_01_init(self):
        self.get_depspecs()

    def test_02_create_error(self):
        self.get_depspecs()
        self.assertRaises(Exception, DepSpec)
        self.assertRaises(Exception, PackageDepSpec)
        self.assertRaises(Exception, StringDepSpec)
        self.assertRaises(BadVersionOperatorError, parse_user_package_dep_spec,
                "<>foo/bar", self.env, UserPackageDepSpecOptions())
        self.assertRaises(PackageDepSpecError, parse_user_package_dep_spec,
                "=foo/bar", self.env, [])
        self.assertRaises(PackageDepSpecError, parse_user_package_dep_spec,
                "*/*::testrepo", self.env, UserPackageDepSpecOptions())
        self.assertRaises(PackageDepSpecError, parse_user_package_dep_spec,
                "*/*::testrepo", self.env, [])

    def test_03_str(self):
        self.get_depspecs()
        self.assertEqual(str(self.ptds), "foo")
        self.assertEqual(str(self.pds), ">=foo/bar-1:100::testrepo")
        self.assertEqual(str(self.bds.blocking), ">=foo/bar-1:100::testrepo")
        self.assertEqual(str(self.nds), "system")

    def test_04_slot(self):
        self.get_depspecs()
        self.assert_(isinstance(self.pds.slot_requirement, SlotExactRequirement))
        self.assertEqual(str(self.pds.slot_requirement.slot), "100")

    def test_05_package(self):
        self.get_depspecs()
        self.assertEqual(str(self.pds.package), "foo/bar")

    def test_06_in_from_repository(self):
        self.get_depspecs()
        self.assertEqual(str(self.pds.in_repository), "testrepo")
        self.assertEqual(self.pds.from_repository, None)

    def test_07_version_requirements(self):
        self.get_depspecs()
        vrc = self.pds.version_requirements

        self.assertEquals(len(list(vrc)), 1)
        self.assertEquals(iter(vrc).next().version_spec, VersionSpec("1"))
        self.assertEquals(iter(vrc).next().version_operator.value, VersionOperator(">=").value)

    def test_08_version_requirements_mode(self):
        self.get_depspecs()
        self.assertEquals(self.pds.version_requirements_mode, VersionRequirementsMode.AND)

###    def test_09_additional_requirements(self):
###        spec = parse_user_package_dep_spec("foo/monkey[foo]", UserPackageDepSpecOptions())
###        ur = iter(spec.additional_requirements).next()
###        self.assert_(isinstance(ur, EnabledUseRequirement))

    def test_11_name(self):
        self.get_depspecs()
        self.assertEqual(str(self.nds.text), "system")

###    def test_11_composites(self):
###        eapi = EAPIData.instance.eapi_from_string("0")
###        spec = PortageDepParser.parse_depend("|| ( foo/bar foo/baz ) foo/monkey", eapi)
###
###        self.assert_(isinstance(spec, CompositeDepSpec))
###        self.assert_(isinstance(spec, AllDepSpec))
###
###        self.assertEqual(len(list(spec)), 2)
###
###        for i, subspec1 in enumerate(spec):
###            if i == 0:
###                self.assert_(isinstance(subspec1, AnyDepSpec))
###                for j, subspec2 in enumerate(subspec1):
###                    if j == 0:
###                        self.assert_(isinstance(subspec2, PackageDepSpec))
###                        self.assertEquals(str(subspec2), "foo/bar")
###                    elif j == 1:
###                        self.assert_(isinstance(subspec2, PackageDepSpec))
###                        self.assertEquals(str(subspec2), "foo/baz")
###                    else:
###                        self.assertEquals("Too many items", "OK")
###            elif i == 1:
###                self.assert_(isinstance(subspec1, PackageDepSpec))
###                self.assertEquals(str(subspec1), "foo/monkey")
###            else:
###                self.assertEquals("Too many items", "OK")

if __name__ == "__main__":
    unittest.main()
