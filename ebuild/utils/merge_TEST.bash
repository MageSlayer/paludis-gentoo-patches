#!/bin/bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006 Ciaran McCreesh <ciaranm@ciaranm.org>
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
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/empty_src" \
        "merge_TEST_dir/empty_dst" \
        "merge_TEST_dir/empty_contents" 1>/dev/null
    test_return_code
}

merge_files_TEST()
{
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/files_src" \
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
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/dirs_src" \
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
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/dirs_over_src" \
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
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/links_src" \
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
    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/links_over_src" \
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
    ! ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/links_over_dir_src" \
        "merge_TEST_dir/links_over_dir_dst" \
        "merge_TEST_dir/links_over_dir_contents" &>/dev/null
    test_return_code
}

merge_config_protect_TEST()
{
    export CONFIG_PROTECT=/dir

    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/config_pro_src" \
        "merge_TEST_dir/config_pro_dst" \
        "merge_TEST_dir/config_pro_contents" 1>/dev/null
    test_return_code

    echo -n "[one]"

    [[ -f merge_TEST_dir/config_pro_dst/dir/one ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0000_one ]] ; test_return_code
    ! [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0001_one ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/one)" "i am a fish"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0000_one)" "contents of one"

    echo -n "[two]"

    [[ -f merge_TEST_dir/config_pro_dst/dir/two ]] ; test_return_code
    ! [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0000_two ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/two)" "contents of two"

    echo -n "[three]"

    [[ -f merge_TEST_dir/config_pro_dst/dir/three ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0000_three ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0001_three ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0002_three ]] ; test_return_code
    ! [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0003_three ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/three)" "i am a fish"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0000_three)" "i am a dish"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0001_three)" "i am a fist"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0002_three)" "contents of three"

    echo -n "[four]"

    [[ -f merge_TEST_dir/config_pro_dst/dir/four ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0000_four ]] ; test_return_code
    [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0001_four ]] ; test_return_code
    ! [[ -f merge_TEST_dir/config_pro_dst/dir/._cfg0002_four ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/four)" "i am a fish"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0000_four)" "contents of four"
    test_equality "$(< merge_TEST_dir/config_pro_dst/dir/._cfg0001_four)" "i am a fist"
}

merge_config_protect_noroot_TEST()
{
    export CONFIG_PROTECT=$(${PALUDIS_EBUILD_DIR}/utils/canonicalise `pwd` )/merge_TEST_dir/noroot_dst/dir

    ${TOP_BUILD_DIR}/ebuild/utils/merge "merge_TEST_dir/config_pro_noroot_src" \
        "/" \
        "merge_TEST_dir/config_pro_slash_root_contents" 1>/dev/null
    test_return_code

    echo -n "[one]"

    [[ -f merge_TEST_dir/noroot_dst/dir/one ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0000_one ]] ; test_return_code
    ! [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0001_one ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/one)" "i am a fish"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0000_one)" "contents of one"

    echo -n "[two]"

    [[ -f merge_TEST_dir/noroot_dst/dir/two ]] ; test_return_code
    ! [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0000_two ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/two)" "contents of two"

    echo -n "[three]"

    [[ -f merge_TEST_dir/noroot_dst/dir/three ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0000_three ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0001_three ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0002_three ]] ; test_return_code
    ! [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0003_three ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/three)" "i am a fish"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0000_three)" "i am a dish"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0001_three)" "i am a fist"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0002_three)" "contents of three"

    echo -n "[four]"

    [[ -f merge_TEST_dir/noroot_dst/dir/four ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0000_four ]] ; test_return_code
    [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0001_four ]] ; test_return_code
    ! [[ -f merge_TEST_dir/noroot_dst/dir/._cfg0002_four ]] ; test_return_code
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/four)" "i am a fish"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0000_four)" "contents of four"
    test_equality "$(< merge_TEST_dir/noroot_dst/dir/._cfg0001_four)" "i am a fist"

}

