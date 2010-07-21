#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir pretend_TEST_dir || exit 1
cd pretend_TEST_dir || exit 1
mkdir -p build

mkdir -p config/.paludis-pretend-test
cat <<END > config/.paludis-pretend-test/specpath.conf
root = `pwd`/root
END

mkdir -p root/${SYSCONFDIR}/paludis/repositories
cat <<END > root/${SYSCONFDIR}/paludis/use.conf
*/* foo
END

cat <<END > root/${SYSCONFDIR}/paludis/general.conf
world = `pwd`/root/world
END

cat <<END > root/${SYSCONFDIR}/paludis/licenses.conf
*/* *
END

cat <<END > root/${SYSCONFDIR}/paludis/keywords.conf
*/* test
END

cat <<END > root/${SYSCONFDIR}/paludis/bashrc
export CHOST="my-chost"
export USER_BASHRC_WAS_USED=yes
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/repo1.conf
location = `pwd`/repo1
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/profile
builddir = `pwd`/build
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/installed.conf
location = `pwd`/root/var/db/pkg
format = vdb
names_cache = /var/empty
provides_cache = /var/empty
builddir = `pwd`/build
END
mkdir -p root/tmp
mkdir -p root/var/db/pkg
mkdir -p root/var/db/exndbam
touch root/${SYSCONFDIR}/ld.so.conf

mkdir -p repo1/{eclass,distfiles,profiles/profile,test-category/target/files} || exit 1

cd repo1 || exit 1
echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
test-category
END
cat <<END > profiles/profile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
USE_EXPAND="USERLAND KERNEL"
END

cat <<"END" > test-category/target/target-1.exheres-0 || exit 1
DESCRIPTION="Test target"
HOMEPAGE="http://paludis.pioto.org/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"

pkg_pretend() {
    [[ ${PRETEND_SHOULD_FAIL} == yes ]] && die "pretend failure"
}

END
cd ..

