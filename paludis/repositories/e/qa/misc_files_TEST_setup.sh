#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir misc_files_TEST_dir || exit 2
cd misc_files_TEST_dir || exit 3

mkdir "cat" || exit 4

mkdir "cat/yes" || exit 5
touch "cat/yes/metadata.xml" || exit 6
mkdir "cat/yes/files" || exit 7

mkdir "cat/no-metadata" || exit 8
mkdir "cat/no-metadata/files" || exit 10

mkdir "cat/bad-files" || exit 11
touch "cat/bad-files/metadata.xml" "cat/bad-files/files"|| exit 12

