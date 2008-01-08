#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2008 Ciaran McCreesh
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be optionful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

option_TEST()
{
    export OPTIONS="foo bar" MYOPTIONS="foo bar baz foofoo fo"

    option foo ; test_return_code
    option bar ; test_return_code
    ! option baz ; test_return_code
    ! option foofoo ; test_return_code
    ! option fo ; test_return_code

    ! option !foo ; test_return_code
    ! option !bar ; test_return_code
    option !baz ; test_return_code
    option !foofoo ; test_return_code
    option !fo ; test_return_code
}

optionv_TEST()
{
    export OPTIONS="foo bar" MYOPTIONS="foo bar baz"
    optionv foo 1>/dev/null ; test_return_code
    test_equality "$(optionv foo)" "foo"

    optionv bar 1>/dev/null ; test_return_code
    test_equality "$(optionv bar)" "bar"

    ! optionv baz 1>/dev/null ; test_return_code
    test_equality "$(optionv baz)" ""

    ! optionv !foo 1>/dev/null ; test_return_code
    test_equality "$(optionv !foo)" ""

    ! optionv !bar 1>/dev/null ; test_return_code
    test_equality "$(optionv !bar)" ""

    ! optionv baz 1>/dev/null ; test_return_code
    test_equality "$(optionv !baz)" "baz"
}

optionq_TEST()
{
    export option="foo bar" MYOPTIONS="foo bar baz foofoo fo"

    optionq foo ; test_return_code
    optionq bar ; test_return_code
    ! optionq baz ; test_return_code
    ! optionq foofoo ; test_return_code
    ! optionq fo ; test_return_code

    ! optionq !foo ; test_return_code
    ! optionq !bar ; test_return_code
    optionq !baz ; test_return_code
    optionq !foofoo ; test_return_code
    optionq !fo ; test_return_code
}

