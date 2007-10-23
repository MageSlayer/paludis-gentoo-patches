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
        self.e = EnvironmentMaker.instance.make_from_spec("")
        self.db = self.e.package_database

    def test_01_user_mask(self):
        q = Query.Matches(PackageDepSpec("=masked/user-1.0", PackageDepSpecParseMode.PERMISSIVE))
        pid = iter(self.db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UserMask))

        self.assertEquals(m.key(), "U")
        self.assertEquals(m.description(), "user")

    def test_02_unaccepted_mask(self):
        q = Query.Matches(PackageDepSpec("=masked/unaccepted-1.0", PackageDepSpecParseMode.PERMISSIVE))
        pid = iter(self.db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnacceptedMask))

        self.assertEquals(m.key(), "K")
        self.assertEquals(m.description(), "keywords")
        self.assert_(isinstance(m.unaccepted_key(), MetadataKeywordNameIterableKey))

    def test_03_repository_mask(self):
        q = Query.Matches(PackageDepSpec("=masked/repo-1.0", PackageDepSpecParseMode.PERMISSIVE))
        pid = iter(self.db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, RepositoryMask))

        self.assertEquals(m.key(), "R")
        self.assertEquals(m.description(), "repository")

        for section in self.db.fetch_repository("testrepo").info(False).sections:
            for k, v in section.kvs:
                if k == "location":
                    location = v
        package_mask_path = os.path.join(location, "profiles/package.mask")
        self.assertEquals(m.mask_key().value().mask_file, package_mask_path)
        self.assert_(isinstance(m.mask_key().value().comment, StringIterable))

    def test_04_unsupported_mask(self):
        q = Query.Matches(PackageDepSpec("=masked/unsupported-1.0", PackageDepSpecParseMode.PERMISSIVE))
        pid = iter(self.db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, UnsupportedMask))

        self.assertEquals(m.key(), "E")
        self.assertEquals(m.description(), "eapi")
        self.assertEquals(m.explanation(), "Unsupported EAPI 'unsupported'")

    def test_05_association_mask(self):
        q = Query.Matches(PackageDepSpec("=virtual/association-1.0", PackageDepSpecParseMode.PERMISSIVE))
        pid = iter(self.db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
        m = iter(pid.masks).next()

        self.assert_(isinstance(m, Mask))
        self.assert_(isinstance(m, AssociationMask))

        self.assertEquals(m.key(), "A")
        self.assertEquals(m.description(), "by association")
        self.assertEquals(m.associated_package().name, "masked/repo")

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

            def unaccepted_key(self):
                return MetadataStringKey("raw", "human", MetadataKeyType.NORMAL)

        test_unaccepted_mask(TestUnacceptedMask())

    def test_03_repository_mask(self):
        class TestRepositoryMask(RepositoryMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def mask_key(self):
                return MetadataStringKey("raw", "human", MetadataKeyType.NORMAL)

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

    def test_05_association_mask(self):
        class TestAssociationMask(AssociationMask):
            def key(self):
                return "T"

            def description(self):
                return "test"

            def associated_package(self):
                e = EnvironmentMaker.instance.make_from_spec("")
                db = e.package_database
                q = Query.Matches(PackageDepSpec("=masked/user-1.0", PackageDepSpecParseMode.PERMISSIVE))
                pid = iter(db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()
                return pid

        test_association_mask(TestAssociationMask())


if __name__ == "__main__":
    unittest.main()
