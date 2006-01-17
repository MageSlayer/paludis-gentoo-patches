#!/bin/sh
# vim: set ft=sh sw=4 sts=4 et :

mkdir fs_entry_TEST_dir || exit 2
cd fs_entry_TEST_dir || exit 3
mkdir dir_a || exit 4
ln -s dir_a symlink_to_dir_a || exit 5
touch dir_a/file_in_a || exit 6

