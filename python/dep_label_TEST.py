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

class TestCase_01_URILabels(unittest.TestCase):
    def setUp(self):
        self.list = []
        self.list.append(URIMirrorsThenListedLabel("foo"))
        self.list.append(URIMirrorsOnlyLabel("foo"))
        self.list.append(URIListedOnlyLabel("foo"))
        self.list.append(URIListedThenMirrorsLabel("foo"))
        self.list.append(URILocalMirrorsOnlyLabel("foo"))
        self.list.append(URIManualOnlyLabel("foo"))

    def test_01_no_create(self):
        self.assertRaises(Exception, URILabel)

    def test_02_inheritance(self):
        for l in self.list:
            self.assert_(isinstance(l, URILabel))

    def test_03_text(self):
        for l in self.list:
            self.assert_(l.text, "foo")

    def test_04_str(self):
        for l in self.list:
            self.assert_(str(l), "foo")

class TestCase_02_DependencyLabels(unittest.TestCase):
    def setUp(self):
        self.list = []
        self.list.append((DependencyHostLabel("foo"), DependencySystemLabel))
        self.list.append((DependencyTargetLabel("foo"), DependencySystemLabel))
        self.list.append((DependencyBuildLabel("foo"), DependencyTypeLabel))
        self.list.append((DependencyRunLabel("foo"), DependencyTypeLabel))
        self.list.append((DependencyInstallLabel("foo"), DependencyTypeLabel))
        self.list.append((DependencyCompileLabel("foo"), DependencyTypeLabel))
        self.list.append((DependencySuggestedLabel("foo"), DependencySuggestLabel))
        self.list.append((DependencyRecommendedLabel("foo"), DependencySuggestLabel))
        self.list.append((DependencyRequiredLabel("foo"), DependencySuggestLabel))
        self.list.append((DependencyAnyLabel("foo"), DependencyABIsLabel))
        self.list.append((DependencyMineLabel("foo"), DependencyABIsLabel))
        self.list.append((DependencyPrimaryLabel("foo"), DependencyABIsLabel))
        self.list.append((DependencyABILabel("foo"), DependencyABIsLabel))

    def test_01_no_create(self):
        self.assertRaises(Exception, DependencyLabel)
        self.assertRaises(Exception, DependencySystemLabel)
        self.assertRaises(Exception, DependencyTypeLabel)
        self.assertRaises(Exception, DependencySuggestLabel)
        self.assertRaises(Exception, DependencyABIsLabel)

    def test_02_inheritance(self):
        for t in self.list:
            self.assert_(isinstance(t[0], DependencyLabel))
            self.assert_(isinstance(t[0], t[1]))

    def test_03_text(self):
        for t in self.list:
            self.assert_(t[0].text, "foo")

    def test_04_str(self):
        for t in self.list:
            self.assert_(str(t[0]), "foo")


if __name__ == "__main__":
    unittest.main()
