#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir gpg_TEST_dir || exit 2
cd gpg_TEST_dir || exit 3

mkdir "cat" || exit 4
mkdir "cat/not-signed" || exit 5
touch "cat/not-signed/Manifest" || exit 6
