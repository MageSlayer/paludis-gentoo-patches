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

import os

repo_path = os.path.join(os.getcwd(), "query_TEST_dir/testrepo")

from paludis import *
import unittest

class TestCase_01_Queries(unittest.TestCase):
    def test_1_create(self):
        self.queries = []
        self.queries.append(Query.SupportsInstallAction())
        self.queries.append(Query.SupportsUninstallAction())
        self.queries.append(Query.SupportsInstalledAction())
        self.queries.append(Query.SupportsPretendAction())
        self.queries.append(Query.SupportsConfigAction())
        self.queries.append(Query.InstalledAtRoot("/bar"))
        self.queries.append(Query.NotMasked())
        self.queries.append(Query.Package("foo/bar"))
        self.queries.append(Query.Matches(PackageDepSpec(">=foo/bar-1", PackageDepSpecParseMode.PERMISSIVE)))
        self.queries.append(Query.All())
        self.queries.append(Query.Category("cat-foo"))
        self.queries.append(Query.Repository("foo_repo"))

    def test_2_create_error(self):
        self.assertRaises(Exception, Query)

    def test_3_subclass(self):
        self.test_1_create()
        for query in self.queries:
            self.assert_(isinstance(query, Query))

    def test_4_and(self):
        self.test_1_create()
        length = len(self.queries)
        for i in xrange(length):
            for j in xrange(length):
                self.assert_(isinstance(self.queries[i] & self.queries[j], Query))

class TestCase_02_QueryBase(unittest.TestCase):
    class TestQuery(QueryBase):
        def __init__(self):
            QueryBase.__init__(self)

        def as_human_readable_string(self):
            return "TestQuery"

        def categories(self, e, repos):
            return None

        def packages(self, e, repos, cats):
            return QueryBase.packages(self, e, repos, cats)

        def ids(self, e, repos, pkgs):
            r = e.package_database.fetch_repository("testrepo")
            return list(r.package_ids(iter(pkgs).next()))

    def test_1_create(self):
        e = NoConfigEnvironment(repo_path, "/var/empty")
        db = e.package_database
        q = self.TestQuery()
        pid = iter(db.query(q, QueryOrder.REQUIRE_EXACTLY_ONE)).next()

        self.assertEquals(pid.name, "cat/package")

if __name__ == "__main__":
    unittest.main()
