#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 Ciaran McCreesh
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

wrapped_getfsize_TEST()
{
    mkdir -p wrapped_getfsize_TEST_dir ; test_return_code

    echo -n "12345" > wrapped_getfsize_TEST_dir/five
    echo "12345" > wrapped_getfsize_TEST_dir/six

    ${PALUDIS_EBUILD_DIR}/utils/wrapped_getfsize wrapped_getfsize_TEST_dir/five >/dev/null ; test_return_code
    ${PALUDIS_EBUILD_DIR}/utils/wrapped_getfsize wrapped_getfsize_TEST_dir/six >/dev/null ; test_return_code

    [[ $(${PALUDIS_EBUILD_DIR}/utils/wrapped_getfsize wrapped_getfsize_TEST_dir/five ) == "5" ]] ; test_return_code
    [[ $(${PALUDIS_EBUILD_DIR}/utils/wrapped_getfsize wrapped_getfsize_TEST_dir/six ) == "6" ]] ; test_return_code

    rm -fr wrapped_getfsize_TEST_dir
}

wrapped_getfsize_fail_TEST()
{
    mkdir -p wrapped_getfsize_TEST_dir ; test_return_code

    ! ${PALUDIS_EBUILD_DIR}/utils/wrapped_getfsize wrapped_getfsize_TEST_dir/seven 2>/dev/null ; test_return_code

    rm -fr wrapped_getfsize_TEST_dir
}


