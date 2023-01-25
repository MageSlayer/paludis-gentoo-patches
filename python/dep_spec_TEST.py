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
        self.pds = parse_user_package_dep_spec(
            ">=foo/bar-1:100::testrepo", self.env, []
        )
        self.pds2 = parse_user_package_dep_spec(
            "*/*::testrepo",
            self.env,
            UserPackageDepSpecOptions() + UserPackageDepSpecOption.ALLOW_WILDCARDS,
        )
        self.pds3 = parse_user_package_dep_spec(
            "*/*::testrepo", self.env, [UserPackageDepSpecOption.ALLOW_WILDCARDS]
        )
        self.pds4 = parse_user_package_dep_spec("cat/pkg::testrepo", self.env, [])
        self.pds5 = parse_user_package_dep_spec(
            ">=foo/bar-1:3/4::testrepo", self.env, []
        )
        self.bds = BlockDepSpec("!>=foo/bar-1:100::testrepo", self.pds)
        self.nds = NamedSetDepSpec("system")

    def test_01_init(self):
        self.get_depspecs()

    def test_02_create_error(self):
        self.get_depspecs()
        self.assertRaises(Exception, DepSpec)
        self.assertRaises(Exception, PackageDepSpec)
        self.assertRaises(Exception, StringDepSpec)
        self.assertRaises(
            BadVersionOperatorError,
            parse_user_package_dep_spec,
            "<>foo/bar",
            self.env,
            UserPackageDepSpecOptions(),
        )
        self.assertRaises(
            PackageDepSpecError, parse_user_package_dep_spec, "=foo/bar", self.env, []
        )
        self.assertRaises(
            PackageDepSpecError,
            parse_user_package_dep_spec,
            "*/*::testrepo",
            self.env,
            UserPackageDepSpecOptions(),
        )
        self.assertRaises(
            PackageDepSpecError,
            parse_user_package_dep_spec,
            "*/*::testrepo",
            self.env,
            [],
        )

    def test_03_str(self):
        self.get_depspecs()
        self.assertEqual(str(self.ptds), "foo")
        self.assertEqual(str(self.pds), ">=foo/bar-1:100::testrepo")
        self.assertEqual(str(self.bds.blocking), ">=foo/bar-1:100::testrepo")
        self.assertEqual(str(self.nds), "system")

    def test_04_slot(self):
        self.get_depspecs()
        self.assertTrue(
            isinstance(self.pds.slot_requirement, SlotExactPartialRequirement)
        )
        self.assertEqual(str(self.pds.slot_requirement.slot), "100")

        self.assertEqual(self.pds2.slot_requirement, None)

        self.assertTrue(
            isinstance(self.pds5.slot_requirement, SlotExactFullRequirement)
        )
        self.assertEqual(str(self.pds5.slot_requirement.slots[0]), "3")
        self.assertEqual(str(self.pds5.slot_requirement.slots[1]), "4")

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

        self.assertEqual(len(list(vrc)), 1)
        self.assertEqual(next(iter(vrc)).version_spec, VersionSpec("1"))
        self.assertEqual(
            next(iter(vrc)).version_operator.value, VersionOperator(">=").value
        )

    def test_08_version_requirements_mode(self):
        self.get_depspecs()
        self.assertEqual(
            self.pds.version_requirements_mode, VersionRequirementsMode.AND
        )

    # def test_09_additional_requirements(self):
    #     spec = parse_user_package_dep_spec("foo/monkey[foo]", UserPackageDepSpecOptions())
    #     ur = next(iter(spec.additional_requirements))
    #     self.assertTrue(isinstance(ur, EnabledUseRequirement))

    def test_11_name(self):
        self.get_depspecs()
        self.assertEqual(str(self.nds.text), "system")

    # def test_11_composites(self):
    #     eapi = EAPIData.instance.eapi_from_string("0")
    #     spec = PortageDepParser.parse_depend("|| ( foo/bar foo/baz ) foo/monkey", eapi)
    #
    #     self.assertTrue(isinstance(spec, CompositeDepSpec))
    #     self.assertTrue(isinstance(spec, AllDepSpec))
    #
    #     self.assertEqual(len(list(spec)), 2)
    #
    #     for i, subspec1 in enumerate(spec):
    #         if i == 0:
    #             self.assertTrue(isinstance(subspec1, AnyDepSpec))
    #             for j, subspec2 in enumerate(subspec1):
    #                 if j == 0:
    #                     self.assertTrue(isinstance(subspec2, PackageDepSpec))
    #                     self.assertEqual(str(subspec2), "foo/bar")
    #                 elif j == 1:
    #                     self.assertTrue(isinstance(subspec2, PackageDepSpec))
    #                     self.assertEqual(str(subspec2), "foo/baz")
    #                 else:
    #                     self.assertEqual("Too many items", "OK")
    #         elif i == 1:
    #             self.assertTrue(isinstance(subspec1, PackageDepSpec))
    #             self.assertEqual(str(subspec1), "foo/monkey")
    #         else:
    #             self.assertEqual("Too many items", "OK")


if __name__ == "__main__":
    unittest.main()
