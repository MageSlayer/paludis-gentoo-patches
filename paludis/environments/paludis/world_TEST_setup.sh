#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir world_TEST_dir || exit 2
cd world_TEST_dir || exit 3

cat <<END > world
cat/unchanged
cat/before
cat/alsounchanged
END

