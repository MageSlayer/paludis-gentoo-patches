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
USERLAND="GNU"
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

mkdir -p providestest/{.cache,cat1/{pkg1,pkg2,pkg3}-{1,2}} || exit 1
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/EAPI; do
    echo 0 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-1/SLOT; do
    echo 1 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-2/SLOT; do
    echo 2 >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/USE; do
    echo enabled >${f}
done
for f in providestest/cat1/{pkg1,pkg2,pkg3}-{1,2}/IUSE; do
    echo disabled enabled >${f}
done

echo '            virtual/foo'                >providestest/cat1/pkg1-1/PROVIDE
echo 'enabled?  ( virtual/foo )'              >providestest/cat1/pkg1-2/PROVIDE
echo 'enabled?  ( virtual/foo ) virtual/bar'  >providestest/cat1/pkg2-1/PROVIDE
echo 'disabled? ( virtual/foo ) virtual/bar'  >providestest/cat1/pkg2-2/PROVIDE
echo 'disabled? ( virtual/foo )'              >providestest/cat1/pkg3-1/PROVIDE
echo ''                                       >providestest/cat1/pkg3-2/PROVIDE

mkdir -p providesincrtest/.cache providesincrtest_src{1,2}/{eclass,profiles/profile,{cat1,cat2}/{pkg1,pkg2}} || exit 1
echo paludis-3 >providesincrtest/.cache/provides
echo installed >>providesincrtest/.cache/provides

cat <<END > providesincrtest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo providesincrtest_src1 >providesincrtest_src1/profiles/repo_name
echo providesincrtest_src2 >providesincrtest_src2/profiles/repo_name
echo cat1 >providesincrtest_src1/profiles/categories
echo cat2 >>providesincrtest_src1/profiles/categories
echo cat1 >providesincrtest_src2/profiles/categories

cat <<END >providesincrtest_src1/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
PROVIDE="enabled? ( virtual/foo ) disabled? ( virtual/bar )"
IUSE="enabled disabled"
END
cp providesincrtest_src1/cat1/pkg1/pkg1-{1,1.1}.ebuild
cp providesincrtest_src1/cat1/pkg1/pkg1-{1,2}.ebuild
cp providesincrtest_src1/cat1/{pkg1/pkg1,pkg2/pkg2}-1.ebuild
cp providesincrtest_src1/cat1/pkg1/pkg1-1.1.ebuild providesincrtest_src2/cat1/pkg1/pkg1-1.1-r0.ebuild

cat <<END >providesincrtest_src2/cat1/pkg1/pkg1-1.ebuild
KEYWORDS="test"
SLOT="\${PV:0:1}"
PROVIDE="enabled? ( virtual/bar ) disabled? ( virtual/foo )"
IUSE="enabled disabled"
END

cat <<END >providesincrtest_src1/cat2/pkg1/pkg1-1.ebuild
EAPI=paludis-1
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/moo"
END
cp providesincrtest_src1/cat2/pkg1/pkg1-{1,2}.ebuild

