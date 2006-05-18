#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p unmerge_TEST_dir/{empty_src,empty_dst} || exit 2

mkdir -p unmerge_TEST_dir/{files_src,files_dst} || exit 3
echo "file one" > unmerge_TEST_dir/files_src/one
echo "file two" > unmerge_TEST_dir/files_src/two
