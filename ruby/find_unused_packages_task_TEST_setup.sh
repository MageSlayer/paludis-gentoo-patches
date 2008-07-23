#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir find_unused_packages_task_TEST_dir || exit 1
cd find_unused_packages_task_TEST_dir || exit 1

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,foo/bar/files,foo/baz/files} || exit 1

cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
foo
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END
cat <<END > profiles/profiles.desc
test testprofile stable
END

cat <<"END" > foo/bar/bar-1.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
END

cat <<"END" > foo/bar/bar-2.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="~test"
END

cat <<"END" > foo/baz/baz-1.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
END

cat <<"END" > foo/baz/baz-2.0.ebuild || exit 1
DESCRIPTION="Test package"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
END

cd ..
