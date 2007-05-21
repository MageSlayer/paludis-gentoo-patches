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

from paludis import *
import unittest

class TestCase_MaskReasons(unittest.TestCase):
    def test_01_init(self):
        MaskReasons()

    def test_02_none(self):
        self.assert_(MaskReasons().none)

    def test_03_any(self):
        self.assert_(not MaskReasons().any)

    def test_04_add(self):
        m1 = MaskReasons()
        m1 = m1 + MaskReason.KEYWORD
        self.assert_(m1[MaskReason.KEYWORD])

    def test_05_iadd(self):
        m1 = MaskReasons()
        m1 += MaskReason.KEYWORD
        self.assert_(m1[MaskReason.KEYWORD])

    def test_06_sub(self):
        m1 = MaskReasons()
        m1 += MaskReason.KEYWORD
        m1 = m1 - MaskReason.KEYWORD
        self.assert_(m1.none)

    def test_07_isub(self):
        m1 = MaskReasons()
        m1 += MaskReason.KEYWORD
        m1 -= MaskReason.KEYWORD
        self.assert_(m1.none)

    def test_08_or(self):
        m1 = MaskReasons()
        m2 = MaskReasons()
        m1 += MaskReason.KEYWORD
        m2 += MaskReason.EAPI
        m3 = m1 | m2
        self.assert_(m3[MaskReason.KEYWORD])
        self.assert_(m3[MaskReason.EAPI])

    def test_09_ior(self):
        m1 = MaskReasons()
        m2 = MaskReasons()
        m1 += MaskReason.KEYWORD
        m2 += MaskReason.EAPI
        m2 |= m1
        self.assert_(m2[MaskReason.KEYWORD])
        self.assert_(m2[MaskReason.EAPI])

    def test_10_subtract(self):
        m1 = MaskReasons()
        m2 = MaskReasons()
        m1 += MaskReason.KEYWORD
        m1 += MaskReason.EAPI
        m2 += MaskReason.EAPI
        m1.subtract(m2)
        self.assert_(m1[MaskReason.KEYWORD])
        self.assert_(not m1[MaskReason.EAPI])


if __name__ == "__main__":
    unittest.main()
