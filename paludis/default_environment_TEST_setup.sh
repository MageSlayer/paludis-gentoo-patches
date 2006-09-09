#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir default_environment_TEST_dir || exit 2
cd default_environment_TEST_dir || exit 3

mkdir -p repo/{profile,profiles,cat-one/pkg-one}
cat <<END > repo/profiles/repo_name
foo
END
cat <<END > repo/profiles/categories
cat-one
END
cat <<END > repo/profile/make.defaults
ARCH="foo"
USE="moo"
USE_EXPAND="EXP MORE_EXP THIRD_EXP"
EXP="one"
MORE_EXP="one"
THIRD_EXP="one"
END

mkdir -p home1/.paludis/repositories
cat <<END > home1/.paludis/use.conf
* foo bar baz -fnord
* EXP: two
>=cat-one/pkg-two-2 THIRD_EXP: two
END
cat <<END > home1/.paludis/keywords.conf
* keyword
END
cat <<END > home1/.paludis/licenses.conf
* keyword
END
cat <<END > home1/.paludis/repositories/foo.conf
format = portage
location = `pwd`/repo
profiles = `pwd`/repo/profile
cache = /var/empty
END

mkdir -p home2/.paludis/repositories
cat <<END > home2/.paludis/use.conf
* -* foo bar baz -fnord
* EXP: -* two
>=cat-one/pkg-two-2 THIRD_EXP: -* two
END
cat <<END > home2/.paludis/keywords.conf
* keyword
END
cat <<END > home2/.paludis/licenses.conf
* *
END
cat <<END > home2/.paludis/repositories/foo.conf
format = portage
location = `pwd`/repo
profiles = `pwd`/repo/profile
cache = /var/empty
END

