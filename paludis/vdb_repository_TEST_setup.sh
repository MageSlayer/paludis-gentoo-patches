#!/bin/sh
# vim: set ft=sh sw=4 sts=4 et :

mkdir vdb_repository_TEST_dir || exit 1
cd vdb_repository_TEST_dir || exit 1

mkdir -p repo1/cat-{one/{pkg-one-1,pkg-both-1},two/{pkg-two-2,pkg-both-2}} || exit 1
