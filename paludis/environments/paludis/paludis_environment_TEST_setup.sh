#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir paludis_environment_TEST_dir || exit 2
cd paludis_environment_TEST_dir || exit 3

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

mkdir -p firstrepo/profiles
cat <<END > firstrepo/profiles/repo_name
first
END
mkdir -p secondrepo/profiles
cat <<END > secondrepo/profiles/repo_name
second
END
mkdir -p thirdrepo/profiles
cat <<END > thirdrepo/profiles/repo_name
third
END
mkdir -p fourthrepo/profiles
cat <<END > fourthrepo/profiles/repo_name
fourth
END
mkdir -p fifthrepo/profiles
cat <<END > fifthrepo/profiles/repo_name
fifth
END

mkdir -p sixthrepo/{profile,profiles,cat-one/pkg-one}
cat <<END > sixthrepo/profiles/repo_name
foo
END
cat <<END > sixthrepo/profiles/categories
cat-one
END
cat <<END > sixthrepo/profile/make.defaults
ARCH="keyword"
USE="foo_c"
USE_EXPAND="FOO_CARDS"
FOO_CARDS="four"
END

mkdir -p home1/.paludis/repositories
cat <<END > home1/.paludis/use.conf
*/* foo bar baz -fnord
*/* EXP: two
>=cat-one/pkg-two-2 THIRD_EXP: two
END
cat <<END > home1/.paludis/keywords.conf
*/* keyword
END
cat <<END > home1/.paludis/licenses.conf
*/* keyword
END
cat <<END > home1/.paludis/repositories/foo.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/repo
profiles = `pwd`/repo/profile
cache = /var/empty
END

mkdir -p home2/.paludis/repositories
cat <<END > home2/.paludis/use.conf
*/* -* foo bar baz -fnord
*/* EXP: -* two
>=cat-one/pkg-two-2 THIRD_EXP: -* two
END
cat <<END > home2/.paludis/keywords.conf
*/* keyword
END
cat <<END > home2/.paludis/licenses.conf
*/* *
END
cat <<END > home2/.paludis/repositories/foo.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/repo
profiles = `pwd`/repo/profile
cache = /var/empty
END

mkdir -p home3/.paludis/repositories
cat <<END > home3/.paludis/use.conf
*/* foo bar baz -fnord
*/* EXP: -* two
>=cat-one/pkg-two-2 THIRD_EXP: -* two
END
cat <<END > home3/.paludis/keywords.conf
*/* keyword
END
cat <<END > home3/.paludis/licenses.conf
*/* *
END
cat <<END > home3/.paludis/repositories/foo.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/repo
profiles = `pwd`/repo/profile
cache = /var/empty
END

mkdir -p home4/.paludis/repositories
cat <<END > home4/.paludis/use.conf
*/* foo
END
cat <<END > home4/.paludis/keywords.conf
*/* keyword
END
cat <<END > home4/.paludis/licenses.conf
*/* *
END
cat <<END > home4/.paludis/repositories/first.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/firstrepo
profiles = `pwd`/repo/profile
cache = /var/empty
master_repository = second
importance = 20
END
cat <<END > home4/.paludis/repositories/second.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/secondrepo
profiles = `pwd`/repo/profile
cache = /var/empty
importance = 10
END
cat <<END > home4/.paludis/repositories/third.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/thirdrepo
profiles = `pwd`/repo/profile
cache = /var/empty
master_repository = second
importance = 8
END
cat <<END > home4/.paludis/repositories/fourth.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/fourthrepo
profiles = `pwd`/repo/profile
cache = /var/empty
importance = 12
END
cat <<END > home4/.paludis/repositories/fifth.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/fifthrepo
profiles = `pwd`/repo/profile
cache = /var/empty
master_repository = second
importance = 5
END

mkdir -p home5/.paludis/repositories
cat <<END > home5/.paludis/use.conf
*/* foo FOO_CARDS: one
cat/one FOO_CARDS: -two three
END
cat <<END > home5/.paludis/keywords.conf
*/* keyword
END
cat <<END > home5/.paludis/licenses.conf
*/* *
END
cat <<END > home5/.paludis/repositories/foo.conf
format = ebuild
names_cache = /var/empty
location = `pwd`/sixthrepo
profiles = `pwd`/sixthrepo/profile
cache = /var/empty
END

