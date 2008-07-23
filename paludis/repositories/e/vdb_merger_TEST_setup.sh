#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir vdb_merger_TEST_dir || exit 2
cd vdb_merger_TEST_dir || exit 3
mkdir CONTENTS || exit 4


mkdir -p config_protect_dir/{image,root} || exit 4

cd config_protect_dir/image

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


mkdir -p file_newline_dir/{image,root} || exit 4
touch file_newline_dir/image/"file
newline" || exit 5

mkdir -p dir_newline_dir/{image,root} || exit 4
mkdir dir_newline_dir/image/"dir
newline" || exit 5

mkdir -p sym_newline_dir/{image,root} || exit 4
ln -s foo sym_newline_dir/image/"sym
newline" || exit 5

mkdir -p sym_target_newline_dir/{image,root} || exit 4
ln -s "foo
bar" sym_target_newline_dir/image/sym_target_newline || exit 5

mkdir -p sym_arrow_dir/{image,root} || exit 4
ln -s bar sym_arrow_dir/image/"sym -> arrow" || exit 5

mkdir -p sym_arrow2_dir/{image,root} || exit 4
mkdir sym_arrow2_dir/image/"dir -> ectory" || exit 5
ln -s bar sym_arrow2_dir/image/"dir -> ectory/sym" || exit 5


for d in *_dir; do
    ln -s ${d} ${d%_dir}
done

