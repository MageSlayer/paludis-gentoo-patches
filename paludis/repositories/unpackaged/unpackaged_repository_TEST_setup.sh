#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir unpackaged_repository_TEST_dir || exit 1
cd unpackaged_repository_TEST_dir || exit 1

mkdir -p pkg
cat <<"END" > pkg/first
This is the first file.
END

mkdir -p under_pkg
cat <<"END" > under_pkg/first
This is also the first file.
END

mkdir -p {under_,}root
mkdir -p {under_,}installed

