#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir has_ebuilds_check_TEST_dir || exit 2
cd has_ebuilds_check_TEST_dir || exit 3

mkdir "cat" || exit 4
mkdir "cat/yes" || exit 5
mkdir "cat/no" || exit 6

touch "cat/yes/yes-1.ebuild" || exit 7

