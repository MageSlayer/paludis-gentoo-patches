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

class TestCase_01_QACheckProperties(unittest.TestCase):
    def test_01_create(self):
        QACheckProperties()

    def test_02_set(self):
        qcp = QACheckProperties()

        self.assert_(qcp.none)

        qcp += QACheckProperty.NEEDS_BUILD
        self.assert_(qcp[QACheckProperty.NEEDS_BUILD])
        qcp += QACheckProperty.NEEDS_NETWORK
        self.assert_(qcp[QACheckProperty.NEEDS_NETWORK])

class TestCase_02_QAMessage(unittest.TestCase):
    def test_01_create(self):
        qm = QAMessage("entry", QAMessageLevel.DEBUG, "name", "message")

    def test_02_data_members(self):
        qm = QAMessage("entry", QAMessageLevel.DEBUG, "name", "message")

        self.assertEquals(qm.entry, "entry")
        self.assertEquals(qm.level, QAMessageLevel.DEBUG)
        self.assertEquals(qm.name, "name")
        self.assertEquals(qm.message, "message")

class TestCase_03_QAReporter(unittest.TestCase):
    import paludis
    if hasattr(paludis, "QAReporter"):
        class PyQAR(QAReporter):
            def __init__(self):
                QAReporter.__init__(self)

            def message(self, msg):
                return 1

    def test_01_create(self):
        QAReporter()

    def test_02_no_message(self):
        self.assertRaises(Exception, QAReporter().message, QAMessageLevel.DEBUG, "foo", "foo")

    def test_03_subclass(self):
        self.assert_(isinstance(self.PyQAR(), QAReporter))

    def test_04_subclass_message(self):
        self.assertEquals(self.PyQAR().message(QAMessage("foo", QAMessageLevel.DEBUG, "foo", "foo")), 1)


if __name__ == "__main__":
    unittest.main()
