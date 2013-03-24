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

class TestCase_Contents(unittest.TestCase):
    def test_01_contents_entry(self):
        self.assertRaises(Exception, ContentsEntry)

    def test_02_file_entry(self):
        e = ContentsFileEntry("/foo", "bar")

        self.assert_(isinstance(e, ContentsEntry))
        self.assertEquals(e.location_key().parse_value(), "/foo")
        self.assertEquals(e.part_key().parse_value(), "bar")

    def test_03_dir_entry(self):
        e = ContentsDirEntry("/foo")

        self.assert_(isinstance(e, ContentsEntry))
        self.assertEquals(e.location_key().parse_value(), "/foo")

    def test_04_other_entry(self):
        e = ContentsOtherEntry("/foo")

        self.assert_(isinstance(e, ContentsEntry))
        self.assertEquals(e.location_key().parse_value(), "/foo")

    def test_07_sym_entry(self):
        e = ContentsSymEntry("/foo", "/blah", "baz")

        self.assert_(isinstance(e, ContentsEntry))
        self.assertEquals(e.location_key().parse_value(), "/foo")
        self.assertEquals(e.target_key().parse_value(), "/blah")
        self.assertEquals(e.part_key().parse_value(), "baz")

    def test_08_contents(self):
        entries = []
        entries.append(ContentsSymEntry("/foo", "/blah", "x"))
        entries.append(ContentsFileEntry("/foo", "x"))
        entries.append(ContentsOtherEntry("/dev/foo"))
        entries.append(ContentsDirEntry("/bar"))

        c = Contents()
        for entry in entries:
            c.add(entry)

        for (i, entry) in enumerate(c):
            self.assertEquals(entry.location_key().parse_value(), entries[i].location_key().parse_value())
            self.assertEquals(type(entry), type(entries[i]))
            if i==0:
                self.assertEquals(entry.target_key().parse_value(), entries[i].target_key().parse_value())
            if i==0 or i==1:
                self.assertEquals(entry.part_key().parse_value(), entries[i].part_key().parse_value())
            if i>3:
                self.assertEquals("TOO MANY ENTRIES", "OK")

if __name__ == "__main__":
    unittest.main()
