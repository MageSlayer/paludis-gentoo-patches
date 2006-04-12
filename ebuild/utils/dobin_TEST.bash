#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

source ${PALUDIS_EBUILD_DIR}/install_functions.bash

dobin_TEST()
{
    mkdir -p dobin_TEST_dir/src dobin_TEST_dir/dst ; test_return_code

    echo "one contents" > dobin_TEST_dir/src/one ; test_return_code
    chmod +x dobin_TEST_dir/src/one ; test_return_code

    echo "two contents" > dobin_TEST_dir/src/two ; test_return_code
    chmod +x dobin_TEST_dir/src/two ; test_return_code

    echo "three contents" > dobin_TEST_dir/src/three ; test_return_code
    chmod +x dobin_TEST_dir/src/three ; test_return_code

    export D=dobin_TEST_dir/dst
    ./dobin dobin_TEST_dir/src/one &>/dev/null ; test_return_code
    ./dobin dobin_TEST_dir/src/two dobin_TEST_dir/src/three &>/dev/null ; test_return_code

    [[ -f dobin_TEST_dir/dst/usr/bin/one ]] ; test_return_code
    [[ -f dobin_TEST_dir/dst/usr/bin/two ]] ; test_return_code
    [[ -f dobin_TEST_dir/dst/usr/bin/three ]] ; test_return_code

    test_equality "$(< dobin_TEST_dir/dst/usr/bin/one)" "one contents"
    test_equality "$(< dobin_TEST_dir/dst/usr/bin/two)" "two contents"
    test_equality "$(< dobin_TEST_dir/dst/usr/bin/three)" "three contents"

    rm -fr dobin_TEST_dir
}

dobin_fail_TEST()
{
    mkdir -p dobin_TEST_dir/src dobin_TEST_dir/dst ; test_return_code
    echo "one contents" > dobin_TEST_dir/src/one ; test_return_code
    echo "two contents" > dobin_TEST_dir/src/two ; test_return_code
    echo "three contents" > dobin_TEST_dir/src/three ; test_return_code

    export D=dobin_TEST_dir/dst
    ! ./dobin dobin_TEST_dir/src/four &>/dev/null ; test_return_code
    ! ./dobin dobin_TEST_dir/src/one dobin_TEST_dir/src/seven &>/dev/null ; test_return_code
    ! ./dobin dobin_TEST_dir/src/eight dobin_TEST_dir/src/one &>/dev/null ; test_return_code

    [[ -f dobin_TEST_dir/dst/usr/bin/one ]] ; test_return_code
    ! [[ -f dobin_TEST_dir/dst/usr/bin/four ]] ; test_return_code
    ! [[ -f dobin_TEST_dir/dst/usr/bin/five ]] ; test_return_code
    ! [[ -f dobin_TEST_dir/dst/usr/bin/six ]] ; test_return_code

    test_equality "$(< dobin_TEST_dir/dst/usr/bin/one)" "one contents"

    rm -fr dobin_TEST_dir
}

