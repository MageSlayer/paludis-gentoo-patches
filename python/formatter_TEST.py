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
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING

class TestCase_01_StringifyFormatter(unittest.TestCase):
    def test_01_create(self):
        StringifyFormatter()

    def test_02_passing_to_cpp(self):
        f = StringifyFormatter()

        test_formatter_string(f)
        test_formmater_keyword_name(f)
        test_formatter_use_flag_name(f)
        test_formatter_iuse_flag(f)
        test_formatter_license_spec_tree(f)
        test_formatter_provide_spec_tree(f)
        test_formatter_dependency_spec_tree(f)
        test_formatter_restrict_spec_tree(f)
        test_formatter_simple_uri_spec_tree(f)
        test_formatter_fetchable_uri_spec_tree(f)

class TestCase_02_PythonFormatter(unittest.TestCase):
    def test_01_create(self):
        PythonFormatter()

    def test_02_passing_to_cpp(self):
        f = PythonFormatter()

        test_formatter_string(f)
        test_formmater_keyword_name(f)
        test_formatter_use_flag_name(f)
        test_formatter_iuse_flag(f)
        test_formatter_license_spec_tree(f)
        test_formatter_provide_spec_tree(f)
        test_formatter_dependency_spec_tree(f)
        test_formatter_restrict_spec_tree(f)
        test_formatter_simple_uri_spec_tree(f)
        test_formatter_fetchable_uri_spec_tree(f)

class TestCase_03_Formatters_suclassing(unittest.TestCase):
    def test_python_can_formats(self):

        class TestCanFormatPlain(CanFormatPlainTextDepSpec):
            def format_plain_text_dep_spec_plain(self, d):
                return str(d)

        class TestCanFormatAcceptable(CanFormatKeywordName):
            def format_keyword_name_plain(self, k):
                return str(k)

            def format_keyword_name_accepted(self, k):
                return str(k)

            def format_keyword_name_unaccepted(self, k):
                return str(k)

        class TestCanFormatUse(CanFormatUseFlagName):
            def format_use_flag_name_plain(self, u):
                return str(u)

            def format_use_flag_name_enabled(self, u):
                return str(u)

            def format_use_flag_name_disabled(self, u):
                return str(u)

            def format_use_flag_name_forced(self, u):
                return str(u)

            def format_use_flag_name_masked(self, u):
                return str(u)

        class TestCanFormatIUse(CanFormatIUseFlag):
            def format_iuse_flag_plain(self, u):
                return str(u)

            def format_iuse_flag_enabled(self, u):
                return str(u)

            def format_iuse_flag_disabled(self, u):
                return str(u)

            def format_iuse_flag_forced(self, u):
                return str(u)

            def format_iuse_flag_masked(self, u):
                return str(u)

            def decorate_iuse_flag_added(self, u, s):
                return str(u) + s

            def decorate_iuse_flag_changed(self, u, s):
                return str(u) + s

        class TestCanFormatPackage(CanFormatPackageDepSpec):
            def format_package_dep_spec_plain(self, pds):
                return str(pds)

            def format_package_dep_spec_installed(self, pds):
                return str(pds)

            def format_package_dep_spec_installable(self, pds):
                return str(pds)

        test_plain_roles(TestCanFormatPlain())
        test_acceptable_roles(TestCanFormatAcceptable())
        test_use_roles(TestCanFormatUse())
        test_iuse_roles(TestCanFormatIUse())
        test_package_roles(TestCanFormatPackage())

    def test_can_space(self):

        class TestCanSpace(CanSpace):
            def newline(self):
                return "\n"

            def indent(self, k):
                return " " * k

        test_can_space(TestCanSpace())

    def test_python_formatter(self):

        class TestFormatter(PythonFormatter):
            def format_plain_text_dep_spec_plain(self, d):
                return str(d)

            def format_keyword_name_plain(self, k):
                return str(k)

            def format_keyword_name_accepted(self, k):
                return str(k)

            def format_keyword_name_unaccepted(self, k):
                return str(k)

            def format_use_flag_name_plain(self, u):
                return str(u)

            def format_use_flag_name_enabled(self, u):
                return str(u)

            def format_use_flag_name_disabled(self, u):
                return str(u)

            def format_use_flag_name_forced(self, u):
                return str(u)

            def format_use_flag_name_masked(self, u):
                return str(u)

            def format_iuse_flag_plain(self, u):
                return str(u)

            def format_iuse_flag_enabled(self, u):
                return str(u)

            def format_iuse_flag_disabled(self, u):
                return str(u)

            def format_iuse_flag_forced(self, u):
                return str(u)

            def format_iuse_flag_masked(self, u):
                return str(u)

            def decorate_iuse_flag_added(self, u, s):
                return str(u) + s

            def decorate_iuse_flag_changed(self, u, s):
                return str(u) + s

            def format_package_dep_spec_plain(self, pds):
                return str(pds)

            def format_package_dep_spec_installed(self, pds):
                return str(pds)

            def format_package_dep_spec_installable(self, pds):
                return str(pds)

            def newline(self):
                return "\n"

            def indent(self, k):
                return " " * k

        f = TestFormatter()

        test_plain_roles(f)
        test_acceptable_roles(f)
        test_use_roles(f)
        test_iuse_roles(f)
        test_package_roles(f)
        test_can_space(f)

        test_formatter_string(f)
        test_formmater_keyword_name(f)
        test_formatter_use_flag_name(f)
        test_formatter_iuse_flag(f)
        test_formatter_license_spec_tree(f)
        test_formatter_provide_spec_tree(f)
        test_formatter_dependency_spec_tree(f)
        test_formatter_restrict_spec_tree(f)
        test_formatter_simple_uri_spec_tree(f)
        test_formatter_fetchable_uri_spec_tree(f)


if __name__ == "__main__":
    unittest.main()
