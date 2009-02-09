#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir safe_ofstream_TEST_dir || exit 2
cd safe_ofstream_TEST_dir || exit 3

touch existing
mkdir existing_dir
touch existing_sym_target
ln -s existing_sym_target existing_sym
touch existing_perm
chmod a-rw existing_perm

