#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir depend_idepend_TEST_dir || exit 1
cd depend_idepend_TEST_dir || exit 1

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
cat <<END > profiles/profile/virtuals
END
cat <<END > profiles/profile/make.defaults
ARCH="test"
KERNEL="linux"
LIBC="glibc"
CHOST="i286-badger-linux-gnu"
END

for e in 7 8 ; do

    mkdir -p "cat/eapi${e}ronly"
    cat <<END > cat/eapi${e}ronly/eapi${e}ronly-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="the/rdepend"
END

    mkdir -p "cat/eapi${e}ionly"
    cat <<END > cat/eapi${e}ionly/eapi${e}ionly-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
IDEPEND="the/idepend"
END

    mkdir -p "cat/eapi${e}both"
    cat <<END > cat/eapi${e}both/eapi${e}both-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
RDEPEND="the/rdepend"
IDEPEND="the/idepend"
END

done

cd ..
