#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir action_TEST_dir || exit 1
cd action_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = e
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/repositories/installed.conf
location = `pwd`/installed
format = vdb
names_cache = /var/empty
builddir = `pwd`
END

cat <<END > home/.paludis/keywords.conf
*/* test
~foo/bar-1 ~test
END

cat <<END > home/.paludis/use.conf
*/* enabled
~foo/bar-1 sometimes_enabled
END

cat <<END > home/.paludis/licenses.conf
*/* *
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar/files} || exit 1
cd testrepo || exit 1

echo "stray" > stray

echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
END

cat <<END > profiles/profiles.desc || exit 1
x86 testprofile stable
END

cat <<END > profiles/package.mask || exit 1
foo1/bar
foo2/bar
END

cat <<END > profiles/testprofile/make.defaults || exit 1
ARCH=test
KERNEL=test
USE="test1 test2 -test5"
END

cat <<END > profiles/testprofile/package.mask || exit 1
foo1/bar
foo3/bar
END

cat <<END > profiles/testprofile/package.use || exit 1
foo/bar -test2 test3
END

cat <<END > profiles/testprofile/use.mask || exit 1
test4
END

cat <<END > profiles/testprofile/package.use.mask || exit 1
foo/bar -test4 test5
END

cat <<END > profiles/testprofile/use.force || exit 1
test6
END

cat <<END > profiles/testprofile/package.use.force || exit 1
foo/bar test7
END

cat <<END > profiles/use.desc || exit 1
test1 - A test use flag
END

cat <<END > profiles/use.local.desc || exit 1
foo/bar:test2 - A test local use flag
END

touch foo/metadata.xml

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI=""
SLOT="0"
IUSE="test1"
LICENSE="GPL-2"
KEYWORDS="test"
END

cat <<"END" > foo/bar/bar-2.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.exherbo.org/"
SRC_URI=""
SLOT="0"
IUSE="test2"
LICENSE="GPL-2"
KEYWORDS="~test"
END
cd ..

mkdir -p installed/cat-one/pkg-one-1 || exit 1

for i in SLOT EAPI; do
    echo "0" >installed/cat-one/pkg-one-1/${i}
done

for i in DEPEND RDEPEND LICENSE INHERITED IUSE PDEPEND PROVIDE; do
    touch installed/cat-one/pkg-one-1/${i}
done

echo "flag1 flag2" >>installed/cat-one/pkg-one-1/USE

cat <<END >installed/cat-one/pkg-one-1/CONTENTS
dir //test
obj /test/test_file de54c26b0678df67aca147575523b3c2 1165250496
sym /test/test_link -> /test/test_file 1165250496
END

