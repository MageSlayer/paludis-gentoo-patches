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

sed_TEST()
{
    mkdir -p sed_TEST_dir ; test_return_code

    cat <<"END" > sed_TEST_dir/before ; test_return_code
one
two
three
END

    ${PALUDIS_EBUILD_DIR}/utils/sed --version >/dev/null ; test_return_code
    ${PALUDIS_EBUILD_DIR}/utils/sed --in-place -e '/\(two\|four\)/s/two/five/' \
        sed_TEST_dir/before ; test_return_code

    cat <<"END" > sed_TEST_dir/after ; test_return_code
one
five
three
END

    cmp sed_TEST_dir/before sed_TEST_dir/after >/dev/null ; test_return_code

    ! ${PALUDIS_EBUILD_DIR}/utils/sed -i -e '/\(two\|four\)/s/two/five/' \
        sed_TEST_dir/doesnotexist 2>/dev/null ; test_return_code

    rm -fr sed_TEST_dir
}

