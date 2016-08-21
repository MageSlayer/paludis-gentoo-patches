#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh
# Copyright (c) 2011 David Leverton
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

use_with_TEST()
{
    export USE="foo"
    test_equality "$(use_with foo )" "--with-foo"
    test_equality "$(use_with foo bar )" "--with-bar"
    test_equality "$(use_with foo bar baz )" "--with-bar=baz"

    export USE="oink"
    test_equality "$(use_with foo )" "--without-foo"
    test_equality "$(use_with foo bar )" "--without-bar"
    test_equality "$(use_with foo bar baz )" "--without-bar"

    export PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT=
    test_equality "$(use_with oink zoink "" )" "--with-zoink"
    export PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT=yes
    test_equality "$(use_with oink zoink "" )" "--with-zoink="
}


use_enable_TEST()
{
    export USE="foo"
    test_equality "$(use_enable foo )" "--enable-foo"
    test_equality "$(use_enable foo bar )" "--enable-bar"
    test_equality "$(use_enable foo bar baz )" "--enable-bar=baz"

    export USE="oink"
    test_equality "$(use_enable foo )" "--disable-foo"
    test_equality "$(use_enable foo bar )" "--disable-bar"
    test_equality "$(use_enable foo bar baz )" "--disable-bar"

    export PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT=
    test_equality "$(use_enable oink zoink "" )" "--enable-zoink"
    export PALUDIS_USE_WITH_ENABLE_EMPTY_THIRD_ARGUMENT=yes
    test_equality "$(use_enable oink zoink "" )" "--enable-zoink="
}

