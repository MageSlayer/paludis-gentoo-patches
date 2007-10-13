#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir unpackaged_repository_TEST_dir || exit 1
cd unpackaged_repository_TEST_dir || exit 1

mkdir -p pkg
cat <<"END" > pkg/first
This is the first file.
END

mkdir -p root

mkdir -p installed/{indices/{categories,packages},data/}

