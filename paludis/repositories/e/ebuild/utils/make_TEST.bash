#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

make_TEST()
{
    mkdir -p make_TEST_dir ; test_return_code

    ${PALUDIS_EBUILD_DIR}/utils/sed -e 's,    ,\t,g' <<"END" > make_TEST_dir/GNUmakefile
A=x
B=$(A:C/$/.1/)

x.1 :
    exit 1

yes : $(B)

no : x.1
END

    ${PALUDIS_EBUILD_DIR}/utils/make --version >/dev/null ; test_return_code
    ${PALUDIS_EBUILD_DIR}/utils/make -C make_TEST_dir yes &>/dev/null ; test_return_code
    ! ${PALUDIS_EBUILD_DIR}/utils/make -C make_TEST_dir no &>/dev/null ; test_return_code

    rm -fr make_TEST_dir
}

