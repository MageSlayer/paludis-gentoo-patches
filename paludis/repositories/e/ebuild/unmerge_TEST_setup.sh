#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p unmerge_TEST_dir/{empty_src,empty_dst} || exit 2

mkdir -p unmerge_TEST_dir/{files_src,files_dst} || exit 3
echo "file one" > unmerge_TEST_dir/files_src/one
echo "file two" > unmerge_TEST_dir/files_src/two

mkdir -p unmerge_TEST_dir/{spaces_src,spaces_dst} || exit 4
echo "file one" > unmerge_TEST_dir/spaces_src/"file one"
mkdir -p unmerge_TEST_dir/spaces_src/"dir two"
echo "file two" > unmerge_TEST_dir/spaces_src/"dir two"/"file two"
ln -s "link three" unmerge_TEST_dir/spaces_src/"link three"

