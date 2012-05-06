#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_5_dir || exit 1
cd e_repository_TEST_5_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

mkdir -p repo/{profiles/profile,metadata,eclass} || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "cat" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH="cheese"
USERLAND="GNU"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
LINGUAS="enabled_en enabled_en_GB enabled_en_GB@UTF-8"
USE_EXPAND="LINGUAS USERLAND"
USE_EXPAND_UNPREFIXED="ARCH"
USE_EXPAND_IMPLICIT="USERLAND ARCH"
USE_EXPAND_VALUES_USERLAND="GNU"
USE_EXPAND_VALUES_ARCH="cheese otherarch"
IUSE_IMPLICIT="build"
END

mkdir -p "cat/required-use-at-most-one-none" || exit 1
cat << 'END' > cat/required-use-at-most-one-none/required-use-at-most-one-none-5.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="5"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 disabled2 disabled3 )"
S="${WORKDIR}"
END

mkdir -p "cat/required-use-at-most-one-one" || exit 1
cat << 'END' > cat/required-use-at-most-one-one/required-use-at-most-one-one-5.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="5"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 enabled2 disabled3 )"
S="${WORKDIR}"
END

mkdir -p "cat/required-use-at-most-one-two" || exit 1
cat << 'END' > cat/required-use-at-most-one-two/required-use-at-most-one-two-5.ebuild || exit 1
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
LICENSE="GPL-2"
KEYWORDS="test"
EAPI="5"
IUSE="disabled1 disabled2 disabled3 enabled1 enabled2 enabled3"
REQUIRED_USE="?? ( disabled1 enabled2 enabled3 )"
S="${WORKDIR}"
END

cd ..
cd ..
