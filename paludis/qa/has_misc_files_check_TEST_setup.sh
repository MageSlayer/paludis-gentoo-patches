#!/bin/sh
# vim: set ft=sh sw=4 sts=4 et :

mkdir has_misc_files_check_TEST_dir || exit 2
cd has_misc_files_check_TEST_dir || exit 3

mkdir "cat" || exit 4

mkdir "cat/yes" || exit 5
touch "cat/yes/ChangeLog" "cat/yes/metadata.xml" || exit 6
mkdir "cat/yes/files" || exit 7

mkdir "cat/no-changelog" || exit 8
touch "cat/no-changelog/metadata.xml" || exit 9
mkdir "cat/no-changelog/files" || exit 10

mkdir "cat/no-metadata" || exit 8
touch "cat/no-metadata/ChangeLog" || exit 9
mkdir "cat/no-metadata/files" || exit 10

mkdir "cat/no-files" || exit 11
touch "cat/no-files/metadata.xml" "cat/no-files/ChangeLog" || exit 12

