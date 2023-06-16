#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p vdb_repository_TEST_cache_dir || exit 1
cd vdb_repository_TEST_cache_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p namesincrtest/.cache/names/installed namesincrtest_src/{eclass,profiles/profile,cat1/{pkg1,pkg2},{cat2,cat3}/pkg1} || exit 1
echo paludis-2 >namesincrtest/.cache/names/installed/_VERSION_
echo installed >>namesincrtest/.cache/names/installed/_VERSION_

cat <<END > namesincrtest_src/profiles/profile/make.defaults
ARCH=test
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo namesincrtest_src >namesincrtest_src/profiles/repo_name
echo cat1 >namesincrtest_src/profiles/categories
echo cat2 >>namesincrtest_src/profiles/categories
echo cat3 >>namesincrtest_src/profiles/categories

cat <<END >namesincrtest_src/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
END
cp namesincrtest_src/cat1/pkg1/pkg1-{1,1.1}.ebuild
cp namesincrtest_src/cat1/pkg1/pkg1-{1,2}.ebuild
cp namesincrtest_src/cat1/{pkg1/pkg1,pkg2/pkg2}-1.ebuild
cp namesincrtest_src/{cat1,cat2}/pkg1/pkg1-1.ebuild

cat <<END >namesincrtest_src/cat3/pkg1/pkg1-1.ebuild
EAPI=paludis-1
KEYWORDS="test"
SLOT="0"
END
cp namesincrtest_src/cat3/pkg1/pkg1-{1,2}.ebuild

