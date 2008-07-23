#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p eselect_env_update_TEST_dir || exit 1
cd eselect_env_update_TEST_dir || exit 1

mkdir -p {etc/env.d,lib,tmp}
touch etc/ld.so.conf
cat <<END > etc/env.d/00basic
LDPATH="/lib"
END

cd ..

