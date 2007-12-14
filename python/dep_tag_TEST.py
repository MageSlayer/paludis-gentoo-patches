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

class TestCase_01_DepTag(unittest.TestCase):
    def test_01_no_create(self):
        self.assertRaises(Exception, DepTag)

class TestCase_02_GLSADepTag(unittest.TestCase):
    def setUp(self):
        global dt
        dt = GLSADepTag("id", "title", "/path")

    def test_01_instance(self):
        self.assert_(isinstance(dt, DepTag))

    def test_02_properties(self):
        self.assertEquals(dt.category, "glsa")
        self.assertEquals(dt.short_text, "GLSA-id")
        self.assertEquals(dt.glsa_title, "title")
        self.assertEquals(dt.glsa_file, "/path")

class TestCase_03_GeneralSetDepTag(unittest.TestCase):
    def setUp(self):
        global dt
        dt = GeneralSetDepTag("set_name", "source")

    def test_01_instance(self):
        self.assert_(isinstance(dt, DepTag))

    def test_02_properties(self):
        self.assertEquals(dt.category, "general")
        self.assertEquals(dt.short_text, "set_name")
        self.assertEquals(dt.source, "source")

class TestCase_04_DependencyDepTag(unittest.TestCase):
    def setUp(self):
        global dt, pid, pds, cds
        env = TestEnvironment()
        repo = FakeRepository(env, "repo")
        pid = repo.add_version("cat/foo", "1.0")
        pds = parse_user_package_dep_spec("=cat/boo-1", [])
        cds = env.set("everything")
        dt = DependencyDepTag(pid, pds, cds);

    def test_01_instance(self):
        self.assert_(isinstance(dt, DepTag))

    def test_02_properties(self):
        self.assertEquals(dt.category, "dependency")
        self.assertEquals(dt.short_text, "cat/foo-1.0:0::repo")
        self.assertEquals(dt.package_id, pid)
        self.assertEquals(str(dt.dependency), str(pds))
        self.assert_(isinstance(dt.conditions, AllDepSpec))

class TestCase_05_TargetDepTag(unittest.TestCase):
    def setUp(self):
        global dt
        dt = TargetDepTag()

    def test_01_instance(self):
        self.assert_(isinstance(dt, DepTag))

    def test_02_properties(self):
        self.assertEquals(dt.category, "target")
        self.assertEquals(dt.short_text, "target")

class TestCase_06_DepTagCategoryMaker(unittest.TestCase):
    def test_01_no_create(self):
        self.assertRaises(Exception, DepTagCategoryMaker)

    def test_02_instance(self):
        self.assert_(isinstance(DepTagCategoryMaker.instance, DepTagCategoryMaker))

    def test_03_make_from_id(self):
        cdt = DepTagCategoryMaker.instance.make_from_id("glsa")
        self.assert_(isinstance(cdt, DepTagCategory))

        self.assertRaises(NoSuchDepTagCategory, DepTagCategoryMaker.instance.make_from_id, "foo")

class TestCase_07_DepTagCategory(unittest.TestCase):
    def setUp(self):
        global cdt
        cdt = DepTagCategoryMaker.instance.make_from_id("glsa")

    def test_01_no_create(self):
        self.assertRaises(Exception, DepTagCategory)

    def test_02_properties(self):
        self.assertEquals(cdt.visible, True)
        self.assertEquals(cdt.id, "glsa")
        self.assertEquals(cdt.title, "Security advisories")
        self.assertEquals(cdt.pre_text, "Your system is potentially affected by these security issues:")
        self.assertEquals(cdt.post_text, "Please read the advisories carefully and take appropriate action.")

if __name__ == "__main__":
    unittest.main()
