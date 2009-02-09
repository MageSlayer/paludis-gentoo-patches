#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir safe_ifstream_TEST_dir || exit 2
cd safe_ifstream_TEST_dir || exit 3

echo first > existing
for (( a = 0 ; a < 1000 ; ++a )) ; do
    echo -n x >> existing
done
echo >> existing

mkdir existing_dir
ln -s existing_sym_target existing
touch existing_perm
chmod a-rw existing_perm

