#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Stephen Bennett
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

KV_major_TEST()
{
    test_equality "$(KV_major 2.4.6)" "2"
}

KV_minor_TEST()
{
    test_equality "$(KV_minor 2.6.16)" "6"
    test_equality "$(KV_minor 2.6.7-gentoo-r1)" "6"
    test_equality "$(KV_minor 2.4.18-rc1)" "4"
}

KV_micro_TEST()
{
    test_equality "$(KV_micro 2.6.0)" "0"
    test_equality "$(KV_micro 2.5.63-mm8)" "63"
    test_equality "$(KV_micro 2.2.0-foo1)" "0"
}

KV_to_int_TEST()
{
    test_equality "$(KV_to_int 2.6.16)" "132624"
}
