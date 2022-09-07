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

repo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/testrepo")
irepo_path = os.path.join(os.getcwd(), "metadata_key_TEST_dir/installed")
ph = os.path.join(os.getcwd(), "metadata_key_TEST_dir/home")
os.environ["PALUDIS_HOME"] = ph

from paludis import *
from additional_tests import *

import unittest

Log.instance.log_level = LogLevel.WARNING


class TestCase_01_MetadataKeys(unittest.TestCase):
    def setUp(self):
        self.e = EnvironmentFactory.instance.create("")
        self.pid = next(
            iter(self.e.fetch_repository("testrepo").package_ids("foo/bar", []))
        )
        self.ipid = next(
            iter(
                self.e.fetch_repository("installed").package_ids("cat-one/pkg-one", [])
            )
        )

    def test_02_installed_time(self):
        self.assertEquals(self.pid.find_metadata("INSTALLED_TIME"), None)
        self.assert_(
            isinstance(self.ipid.find_metadata("INSTALLED_TIME"), MetadataTimeKey)
        )

    def test_03_repository(self):
        self.assertEquals(self.pid.find_metadata("REPOSITORIES"), None)
        self.assert_(
            isinstance(
                self.ipid.find_metadata("REPOSITORIES"), MetadataStringIterableKey
            )
        )

    def test_04_keywords(self):
        self.assert_(
            isinstance(
                self.pid.find_metadata("KEYWORDS"), MetadataKeywordNameIterableKey
            )
        )
        self.assertEquals(self.ipid.find_metadata("KEYWORDS"), None)

    def test_07_inherited(self):
        self.assert_(
            isinstance(self.pid.find_metadata("INHERITED"), MetadataStringIterableKey)
        )
        self.assert_(
            isinstance(self.ipid.find_metadata("INHERITED"), MetadataStringIterableKey)
        )

    def test_08_depend(self):
        self.assert_(
            isinstance(self.pid.find_metadata("DEPEND"), MetadataDependencySpecTreeKey)
        )
        self.assertEquals(self.ipid.find_metadata("DEPEND"), None)

    def test_011_choices(self):
        self.assert_(
            isinstance(self.pid.find_metadata("PALUDIS_CHOICES"), MetadataChoicesKey)
        )
        self.assert_(
            isinstance(self.ipid.find_metadata("PALUDIS_CHOICES"), MetadataChoicesKey)
        )


class TestCase_02_MetadataKeys_suclassing(unittest.TestCase):
    def test_01_package_id(self):
        class TestKey(MetadataPackageIDKey):
            def __init__(self):
                MetadataPackageIDKey.__init__(self)

            def parse_value(self):
                e = EnvironmentFactory.instance.create("")
                pid = next(
                    iter(e.fetch_repository("testrepo").package_ids("foo/bar", []))
                )
                return pid

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_package_id_key(TestKey())

    def test_02_string(self):
        class TestKey(MetadataStringKey):
            def __init__(self):
                MetadataStringKey.__init__(self)

            def parse_value(self):
                return "str"

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_string_key(TestKey())

    def test_03_time(self):
        class TestKey(MetadataTimeKey):
            def __init__(self):
                MetadataTimeKey.__init__(self)

            def parse_value(self):
                return 123

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_time_key(TestKey())

    def test_06_keyword_name_iterable(self):
        class TestKey(MetadataKeywordNameIterableKey):
            def __init__(self):
                MetadataKeywordNameIterableKey.__init__(self)

            def parse_value(self):
                return ["keyword"]

            def pretty_print_flat(self, f):
                return f.format_keyword_name_plain(KeywordName("foo"))

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_keyword_name_set_key(TestKey())

    def test_09_string_iterable(self):
        class TestKey(MetadataStringIterableKey):
            def __init__(self):
                MetadataStringIterableKey.__init__(self)

            def parse_value(self):
                return ["string"]

            def pretty_print_flat(self, f):
                return f.format_string_plain("foo")

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_string_set_key(TestKey())

    def test_10_license_spec_tree(self):
        class TestKey(MetadataLicenseSpecTreeKey):
            def __init__(self):
                MetadataLicenseSpecTreeKey.__init__(self)

            def parse_value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def pretty_print_flat(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_license_spec_tree_key(TestKey())

    def test_12_dependency_spec_tree(self):
        class TestKey(MetadataDependencySpecTreeKey):
            def __init__(self):
                self.e = EnvironmentFactory.instance.create("")
                MetadataDependencySpecTreeKey.__init__(self)

            def parse_value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                pds = parse_user_package_dep_spec("cat/pkg", self.e, [])
                return f.format_package_dep_spec_plain(pds)

            def pretty_print_flat(self, f):
                pds = parse_user_package_dep_spec("cat/pkg", self.e, [])
                return f.format_package_dep_spec_plain(pds)

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_dependency_spec_tree_key(TestKey())

    def test_13_plain_text_spec_tree(self):
        class TestKey(MetadataPlainTextSpecTreeKey):
            def __init__(self):
                MetadataPlainTextSpecTreeKey.__init__(self)

            def parse_value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                # TODO
                # return f.format_plain_text_dep_spec_plain(PlainTextDepSpec("foo"))
                return "str"

            def pretty_print_flat(self, f):
                # TODO
                # return f.format_plain_text_dep_spec_plain(PlainTextDepSpec("foo"))
                return "str"

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_plain_text_spec_tree_key(TestKey())

    def test_14_fetchable_uri_spec_tree(self):
        class TestKey(MetadataFetchableURISpecTreeKey):
            def __init__(self):
                MetadataFetchableURISpecTreeKey.__init__(self)

            def parse_value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def pretty_print_flat(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def initial_label(self):
                return URIMirrorsOnlyLabel("foo")

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_fetchable_uri_spec_tree_key(TestKey())

    def test_15_simple_uri_spec_tree(self):
        class TestKey(MetadataSimpleURISpecTreeKey):
            def __init__(self):
                MetadataSimpleURISpecTreeKey.__init__(self)

            def parse_value(self):
                return AllDepSpec()

            def pretty_print(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def pretty_print_flat(self, f):
                # TODO
                # return f.format_use_dep_spec_plain(...)
                return "str"

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_simple_uri_spec_tree_key(TestKey())

    def test_16_section(self):
        class TestKey(MetadataSectionKey):
            def __init__(self):
                MetadataSectionKey.__init__(self)

            def need_keys_added(self):
                return

            def title_key(self):
                return None

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_section_key(TestKey())

    def test_17_choices(self):
        class TestKey(MetadataChoicesKey):
            def __init__(self):
                MetadataChoicesKey.__init__(self)

            def parse_value(self):
                return Choices()

            def raw_name(self):
                return "raw"

            def human_name(self):
                return "human"

            def type(self):
                return MetadataKeyType.NORMAL

        test_metadata_choices_key(TestKey())


if __name__ == "__main__":
    unittest.main()
