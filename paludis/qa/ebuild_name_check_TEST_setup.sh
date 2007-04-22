#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir ebuild_name_check_TEST_dir || exit 2
cd ebuild_name_check_TEST_dir || exit 3

mkdir "valid-cat" || exit 4
mkdir "valid-cat/valid-pkg" || exit 5
touch "valid-cat/valid-pkg/valid-pkg-0.ebuild" || exit 6
touch "valid-cat/valid-pkg/invalid-pkg-0.ebuild" || exit 7

