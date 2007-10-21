#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

patch_TEST()
{
    mkdir -p patch_TEST_dir ; test_return_code

    cat <<"END" > patch_TEST_dir/a ; test_return_code
one
two
three
four
 five
six
END

    ${PALUDIS_EBUILD_DIR}/utils/patch --version >/dev/null ; test_return_code
    cat <<"END" | ${PALUDIS_EBUILD_DIR}/utils/patch -l -p1 ; test_return_code
What's the deal with the giant space monkeys?
--- old/patch_TEST_dir/a
+++ new/patch_TEST_dir/a
@@ -1,6 +1,6 @@
 one
 two
-three
+ten
 four
-  five
+five
 six
END

    cat <<"END" > patch_TEST_dir/expected ; test_return_code
one
two
ten
four
five
six
END

    cmp patch_TEST_dir/expected patch_TEST_dir/a >/dev/null ; test_return_code

    cat <<"END" | { ! ${PALUDIS_EBUILD_DIR}/utils/patch -l -p1 &>/dev/null ; } ; test_return_code
--- old/patch_TEST_dir/b
+++ new/patch_TEST_dir/b
@@ -1,6 +1,6 @@
 one
 two
-three
+ten
 four
-  five
+five
 six
END

    rm -fr patch_TEST_dir
}

