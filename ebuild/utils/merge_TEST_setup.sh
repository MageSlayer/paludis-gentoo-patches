#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p merge_TEST_dir/{empty_src,empty_dst} || exit 2

mkdir -p merge_TEST_dir/{files_src,files_dst} || exit 2
echo "contents of one" > merge_TEST_dir/files_src/one
echo "contents of two" > merge_TEST_dir/files_src/two

mkdir -p merge_TEST_dir/{dirs_src/{dir_one,dir_two/dir_three},dirs_dst} || exit 3
echo "contents of one" > merge_TEST_dir/dirs_src/dir_one/one
echo "contents of two" > merge_TEST_dir/dirs_src/dir_two/two
echo "contents of three" > merge_TEST_dir/dirs_src/dir_two/dir_three/three

mkdir -p merge_TEST_dir/{dirs_over_src/{dir_one,dir_two/dir_three},dirs_over_dst} || exit 4
mkdir -p merge_TEST_dir/dirs_over_dst/{one,real}
ln -s real merge_TEST_dir/dirs_over_dst/two
echo "contents of one" > merge_TEST_dir/dirs_over_src/dir_one/one
echo "contents of two" > merge_TEST_dir/dirs_over_src/dir_two/two
echo "contents of three" > merge_TEST_dir/dirs_over_src/dir_two/dir_three/three

mkdir -p merge_TEST_dir/{links_src,links_dst} || exit 5
echo "contents of one" > merge_TEST_dir/links_src/one
echo "contents of two" > merge_TEST_dir/links_src/two
ln -s two merge_TEST_dir/links_src/link_to_two

mkdir -p merge_TEST_dir/{links_over_src,links_over_dst} || exit 5
echo "contents of one" > merge_TEST_dir/links_over_src/one
echo "contents of two" > merge_TEST_dir/links_over_src/two
echo "contents of three" > merge_TEST_dir/links_over_src/three
ln -s two merge_TEST_dir/links_over_src/link_to_two
ln -s three merge_TEST_dir/links_over_src/link_to_three
echo "existing file" > merge_TEST_dir/link_to_two
ln -s ../ merge_TEST_dir/links_over_dst/link_to_three

mkdir -p merge_TEST_dir/{links_over_dir_src,links_over_dir_dst} || exit 5
echo "contents of one" > merge_TEST_dir/links_over_dir_src/one
echo "contents of two" > merge_TEST_dir/links_over_dir_src/two
echo "contents of three" > merge_TEST_dir/links_over_dir_src/three
ln -s two merge_TEST_dir/links_over_dir_src/link_to_two
ln -s three merge_TEST_dir/links_over_dir_src/link_to_three
mkdir merge_TEST_dir/links_over_dir_dst/link_to_two
echo "existing file" > merge_TEST_dir/links_over_dir_dst/link_to_three

