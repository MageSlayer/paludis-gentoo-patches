#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

unmerge_empty_TEST()
{
    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/merge "unmerge_TEST_dir/empty_src" \
        "unmerge_TEST_dir/empty_dst" \
        "unmerge_TEST_dir/empty_contents" 1>/dev/null
    test_return_code

    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/unmerge "unmerge_TEST_dir/empty_dst" \
        "unmerge_TEST_dir/empty_contents" 1>/dev/null
    test_return_code

    ok=
    for a in unmerge_TEST_dir/empty_dst/* ; do
        [[ -e "$a" ]] || continue
        test_equality "$a" ""
        ok=no
    done
    test_equality "$ok" ""
}

unmerge_files_TEST()
{
    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/merge "unmerge_TEST_dir/files_src" \
        "unmerge_TEST_dir/files_dst" \
        "unmerge_TEST_dir/files_contents" 1>/dev/null
    test_return_code

    ok=
    for a in unmerge_TEST_dir/files_dst/* ; do
        [[ -e "$a" ]] || continue
        ok=yes
        break
    done
    test_equality "$ok" "yes"

    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/unmerge "unmerge_TEST_dir/files_dst" \
        "unmerge_TEST_dir/files_contents" 1>/dev/null
    test_return_code

    ok=
    for a in unmerge_TEST_dir/files_dst/* ; do
        [[ -e "$a" ]] || continue
        test_equality "$a" ""
        ok=no
    done
    test_equality "$ok" ""
}

unmerge_spaces_TEST()
{
    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/merge "unmerge_TEST_dir/spaces_src" \
        "unmerge_TEST_dir/spaces_dst" \
        "unmerge_TEST_dir/spaces_contents"
    test_return_code

    ok=
    for a in unmerge_TEST_dir/spaces_dst/* ; do
        [[ -e "$a" ]] || continue
        ok=yes
        break
    done
    test_equality "$ok" "yes"

    ${TOP_BUILD_DIR}/paludis/repositories/e/ebuild/unmerge "unmerge_TEST_dir/spaces_dst" \
        "unmerge_TEST_dir/spaces_contents"
    test_return_code

    ok=
    for a in unmerge_TEST_dir/spaces_dst/* ; do
        [[ -e "$a" ]] || continue
        test_equality "$a" ""
        ok=no
    done
    test_equality "$ok" ""
}

