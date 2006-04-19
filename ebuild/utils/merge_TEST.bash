#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

merge_empty_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/empty_src" \
        "merge_TEST_dir/empty_dst" \
        "merge_TEST_dir/empty_contents" 1>/dev/null
    test_return_code
}

merge_files_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/files_src" \
        "merge_TEST_dir/files_dst" \
        "merge_TEST_dir/files_contents" 1>/dev/null
    test_return_code

    [[ -f "merge_TEST_dir/files_dst/one" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/files_dst/one)" "contents of one"

    [[ -f "merge_TEST_dir/files_dst/two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/files_dst/two)" "contents of two"
}

merge_dirs_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/dirs_src" \
        "merge_TEST_dir/dirs_dst" \
        "merge_TEST_dir/dirs_contents" 1>/dev/null
    test_return_code

    [[ -d "merge_TEST_dir/dirs_dst/dir_one" ]] ; test_return_code
    [[ -d "merge_TEST_dir/dirs_dst/dir_two" ]] ; test_return_code
    [[ -d "merge_TEST_dir/dirs_dst/dir_two/dir_three" ]] ; test_return_code

    [[ -f "merge_TEST_dir/dirs_dst/dir_one/one" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_dst/dir_one/one)" "contents of one"

    [[ -f "merge_TEST_dir/dirs_dst/dir_two/two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_dst/dir_two/two)" "contents of two"

    [[ -f "merge_TEST_dir/dirs_dst/dir_two/dir_three/three" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_dst/dir_two/dir_three/three)" "contents of three"
}

merge_dirs_over_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/dirs_over_src" \
        "merge_TEST_dir/dirs_over_dst" \
        "merge_TEST_dir/dirs_over_contents" 1>/dev/null
    test_return_code

    [[ -d "merge_TEST_dir/dirs_over_dst/dir_one/" ]] ; test_return_code
    [[ -d "merge_TEST_dir/dirs_over_dst/dir_two/" ]] ; test_return_code
    [[ -d "merge_TEST_dir/dirs_over_dst/dir_two/dir_three/" ]] ; test_return_code

    [[ -f "merge_TEST_dir/dirs_over_dst/dir_one/one" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_over_dst/dir_one/one)" "contents of one"

    [[ -f "merge_TEST_dir/dirs_over_dst/dir_two/two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_over_dst/dir_two/two)" "contents of two"

    [[ -f "merge_TEST_dir/dirs_over_dst/dir_two/dir_three/three" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/dirs_over_dst/dir_two/dir_three/three)" "contents of three"
}

merge_links_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/links_src" \
        "merge_TEST_dir/links_dst" \
        "merge_TEST_dir/links_contents" 1>/dev/null
    test_return_code

    [[ -f "merge_TEST_dir/links_dst/one" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_dst/one)" "contents of one"

    [[ -f "merge_TEST_dir/links_dst/two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_dst/two)" "contents of two"

    [[ -f "merge_TEST_dir/links_dst/link_to_two" ]] ; test_return_code
    [[ -L "merge_TEST_dir/links_dst/link_to_two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_dst/link_to_two)" "contents of two"
}

merge_links_over_TEST()
{
    ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/links_over_src" \
        "merge_TEST_dir/links_over_dst" \
        "merge_TEST_dir/links_over_contents" 1>/dev/null
    test_return_code

    [[ -f "merge_TEST_dir/links_over_dst/one" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_over_dst/one)" "contents of one"

    [[ -f "merge_TEST_dir/links_over_dst/two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_over_dst/two)" "contents of two"

    [[ -f "merge_TEST_dir/links_over_dst/link_to_two" ]] ; test_return_code
    [[ -L "merge_TEST_dir/links_over_dst/link_to_two" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_over_dst/link_to_two)" "contents of two"

    [[ -f "merge_TEST_dir/links_over_dst/link_to_three" ]] ; test_return_code
    [[ -L "merge_TEST_dir/links_over_dst/link_to_three" ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/links_over_dst/link_to_three)" "contents of three"
}
merge_links_over_dir_TEST()
{
    ! ${PALUDIS_EBUILD_DIR}/utils/merge "merge_TEST_dir/links_over_dir_src" \
        "merge_TEST_dir/links_over_dir_dst" \
        "merge_TEST_dir/links_over_dir_contents" &>/dev/null
    test_return_code
}


