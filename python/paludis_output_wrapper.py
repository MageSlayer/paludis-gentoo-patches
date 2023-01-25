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

import sys

stdout_cache = []
stderr_cache = []


class CommonOutput:
    def __init__(self, out):
        self.out = out

    def writelines(self, seq):
        for s in seq:
            self.write(s)

    def closed(self):
        self.out.closed()

    def close(self):
        self.out.close()

    def isatty(self):
        return self.out.isatty()

    def mode(self):
        return self.out.mode()

    def name(self):
        return self.out.name()

    def encoding(self):
        return self.out.encoding()


class PrefixOutput(CommonOutput):
    def __init__(self, out, prefix):
        CommonOutput.__init__(self, out)

        self.prefix = prefix
        self.need_prefix = True

    def write(self, str):
        if self.need_prefix:
            self.out.write(self.prefix)

        if str.endswith("\n"):
            self.need_prefix = True
            newlines = -1
        else:
            self.need_prefix = False
            newlines = 0

        newlines += str.count("\n")
        self.out.write(str.replace("\n", "\n" + self.prefix, newlines))


def save(what):
    if what == "stderr" or what == "both":
        stderr_cache.append(sys.stderr)
    elif what == "stdout" or what == "both":
        stdout_cache.append(sys.stdout)


def restore(what):
    if what == "stderr" or what == "both":
        if len(stderr_cache):
            sys.stderr = stderr_cache.pop()
        else:
            sys.stderr = sys.__stderr__

    elif what == "stdout" or what == "both":
        if len(stdout_cache):
            sys.stdout = stdout_cache.pop()
        else:
            sys.stdout = sys.__stdout__


def set_prefix(prefix):
    save("both")
    if prefix:
        sys.stdout = PrefixOutput(sys.__stdout__, prefix)
        sys.stderr = PrefixOutput(sys.__stderr__, prefix)
    else:
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__


def restore_prefix():
    restore("both")
