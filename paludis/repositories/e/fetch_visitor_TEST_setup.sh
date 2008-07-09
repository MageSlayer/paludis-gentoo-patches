#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir fetch_visitor_TEST_dir || exit 1
cd fetch_visitor_TEST_dir || exit 1

mkdir -p "in"
mkdir -p "out"

cat <<END > in/input1
contents of one
END

