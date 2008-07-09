#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir stripper_TEST_dir || exit 2
cd stripper_TEST_dir || exit 3

mkdir -p image/usr/bin || exit 5
cp ../stripper_TEST_binary image/usr/bin || exit 6

