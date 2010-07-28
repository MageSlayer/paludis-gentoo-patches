#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir tar_merger_TEST_dir || exit 2
cd tar_merger_TEST_dir || exit 3

mkdir -p simple simple_extract
cat <<END > simple/file
This is the file.
END

