#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir vdb_merger_TEST_dir || exit 2
cd vdb_merger_TEST_dir || exit 3

mkdir -p config_protect/{image,root,CONTENTS} || exit 4

cd config_protect/image

echo foo >protected_file
echo foo >unprotected_file
echo foo >protected_file_not_really

mkdir protected_dir
echo foo >protected_dir/protected_file
echo foo >protected_dir/unprotected_file
echo foo >protected_dir/unprotected_file_not_really

echo foo >protected_dir/protected_file_already_needs_update
echo foo >protected_dir/unchanged_protected_file
echo foo >protected_dir/protected_file_same_as_existing_update

mkdir protected_dir/unprotected_dir
echo foo >protected_dir/unprotected_dir/unprotected_file

mkdir protected_dir/unprotected_dir_not_really
echo foo >protected_dir/unprotected_dir_not_really/protected_file

mkdir protected_dir_not_really
echo foo >protected_dir_not_really/unprotected_file

cd ../root

echo bar >protected_file
echo bar >unprotected_file
echo bar >protected_file_not_really

mkdir protected_dir
echo bar >protected_dir/protected_file
echo bar >protected_dir/unprotected_file
echo bar >protected_dir/unprotected_file_not_really

echo bar >protected_dir/protected_file_already_needs_update
echo baz >protected_dir/._cfg0000_protected_file_already_needs_update
echo foo >protected_dir/unchanged_protected_file
echo bar >protected_dir/protected_file_same_as_existing_update
echo foo >protected_dir/._cfg0000_protected_file_same_as_existing_update

mkdir protected_dir/unprotected_dir
echo bar >protected_dir/unprotected_dir/unprotected_file

mkdir protected_dir/unprotected_dir_not_really
echo bar >protected_dir/unprotected_dir_not_really/protected_file

mkdir protected_dir_not_really
echo bar >protected_dir_not_really/unprotected_file

cd ../..

