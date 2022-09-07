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


class TestCase_Log(unittest.TestCase):
    def test_01_get_instance(self):
        Log.instance

    def test_02_no_init(self):
        self.assertRaises(Exception, Log)

    def test_03_log_level(self):
        ll = Log.instance.log_level

        self.assertEqual(ll, LogLevel.QA)
        self.assert_(ll >= LogLevel.DEBUG)
        self.assert_(ll <= LogLevel.SILENT)

        ll = LogLevel.WARNING
        self.assertEqual(ll, LogLevel.WARNING)

        self.assertRaises(Exception, ll, 123)

    def test_04_program_name(self):
        Log.instance.program_name = "foo"
        self.assertRaises(AttributeError, lambda: Log.instance.program_name)

    def test_05_log_message(self):
        l = Log.instance
        l.log_level = LogLevel.SILENT

        l.message("python.test", LogLevel.DEBUG, LogContext.CONTEXT, "foooo")


if __name__ == "__main__":
    unittest.main()
