#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir package_name_check_TEST_dir || exit 2
cd package_name_check_TEST_dir || exit 3

mkdir "valid-cat" || exit 4
mkdir "valid-cat/valid-pkg" || exit 5
mkdir "valid-cat/invalid-pkg..." || exit 6

mkdir "invalid-cat!" || exit 7
mkdir "invalid-cat!/valid-pkg" || exit 8
mkdir "invalid-cat!/invalid-pkg..." || exit 9

