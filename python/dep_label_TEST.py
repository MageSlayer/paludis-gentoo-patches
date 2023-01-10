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
            self.assertTrue(isinstance(l, URILabel))

    def test_03_text(self):
        for l in self.list:
            self.assertTrue(l.text, "foo")

    def test_04_str(self):
        for l in self.list:
            self.assertTrue(str(l), "foo")


class TestCase_02_DependenciesLabels(unittest.TestCase):
    def setUp(self):
        self.list = []
        self.list.append((DependenciesBuildLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesRunLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesPostLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesCompileAgainstLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesInstallLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesFetchLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesSuggestionLabel("foo"), DependenciesLabel))
        self.list.append((DependenciesRecommendationLabel("foo"), DependenciesLabel))

    def test_01_no_create(self):
        self.assertRaises(Exception, DependenciesLabel)

    def test_02_inheritance(self):
        for t in self.list:
            self.assertTrue(isinstance(t[0], DependenciesLabel))
            self.assertTrue(isinstance(t[0], t[1]))

    def test_03_text(self):
        for t in self.list:
            self.assertTrue(t[0].text, "foo")

    def test_04_str(self):
        for t in self.list:
            self.assertTrue(str(t[0]), "foo")


if __name__ == "__main__":
    unittest.main()
