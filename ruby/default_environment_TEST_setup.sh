#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir default_environment_TEST_dir || exit 1
cd default_environment_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/nothing.conf
format = nothing
location = /var/empty
name = nothing
END

cat <<END > home/.paludis/keywords.conf
* test
END

cat <<END > home/.paludis/use.conf
* enabled
~foo/bar-1 sometimes_enabled
END

cat <<END > home/.paludis/licenses.conf
* *
END

