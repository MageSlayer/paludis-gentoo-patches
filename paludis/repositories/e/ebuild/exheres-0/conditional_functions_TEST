#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2008 Ciaran McCreesh
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

# the real optionq is a pipe command
optionq()
{
    hasq "$1" $OPTIONS
}

option_with_TEST()
{
    export OPTIONS="foo blah:moo" MYOPTIONS="foo bar baz blah:moo"
    test_equality "$(option_with foo )" "--with-foo"
    test_equality "$(option_with foo bar )" "--with-bar"
    test_equality "$(option_with foo bar baz )" "--with-bar=baz"
    test_equality "$(option_with blah:moo )" "--with-moo"

    export OPTIONS="oink"
    test_equality "$(option_with foo )" "--without-foo"
    test_equality "$(option_with foo bar )" "--without-bar"
    test_equality "$(option_with foo bar baz )" "--without-bar"
}


option_enable_TEST()
{
    export OPTIONS="foo blah:moo" MYOPTIONS="foo bar baz blah:moo"
    test_equality "$(option_enable foo )" "--enable-foo"
    test_equality "$(option_enable foo bar )" "--enable-bar"
    test_equality "$(option_enable foo bar baz )" "--enable-bar=baz"
    test_equality "$(option_enable blah:moo )" "--enable-moo"

    export OPTIONS="oink"
    test_equality "$(option_enable foo )" "--disable-foo"
    test_equality "$(option_enable foo bar )" "--disable-bar"
    test_equality "$(option_enable foo bar baz )" "--disable-bar"
}

