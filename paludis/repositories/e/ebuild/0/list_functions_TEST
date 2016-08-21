#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh
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

use_TEST()
{
    export USE="foo bar"

    use foo ; test_return_code
    use bar ; test_return_code
    ! use baz ; test_return_code
    ! use foofoo ; test_return_code
    ! use fo ; test_return_code

    ! use !foo ; test_return_code
    ! use !bar ; test_return_code
    use !baz ; test_return_code
    use !foofoo ; test_return_code
    use !fo ; test_return_code
}

usev_TEST()
{
    export USE="foo bar"
    usev foo 1>/dev/null ; test_return_code
    test_equality "$(usev foo)" "foo"

    usev bar 1>/dev/null ; test_return_code
    test_equality "$(usev bar)" "bar"

    ! usev baz 1>/dev/null ; test_return_code
    test_equality "$(usev baz)" ""

    ! usev !foo 1>/dev/null ; test_return_code
    test_equality "$(usev !foo)" ""

    ! usev !bar 1>/dev/null ; test_return_code
    test_equality "$(usev !bar)" ""

    ! usev baz 1>/dev/null ; test_return_code
    test_equality "$(usev !baz)" "baz"
}

useq_TEST()
{
    export USE="foo bar"

    useq foo ; test_return_code
    useq bar ; test_return_code
    ! useq baz ; test_return_code
    ! useq foofoo ; test_return_code
    ! useq fo ; test_return_code

    ! useq !foo ; test_return_code
    ! useq !bar ; test_return_code
    useq !baz ; test_return_code
    useq !foofoo ; test_return_code
    useq !fo ; test_return_code
}

