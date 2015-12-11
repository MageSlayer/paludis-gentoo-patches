#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_pbin_dir || exit 1
cd e_repository_TEST_pbin_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

for e in 0 1 2 3 4 5 ; do
    mkdir -p repo${e}/{profiles/profile,metadata,eclass} || exit 1
    cd repo${e} || exit 1
    echo "repo${e}" >> profiles/repo_name || exit 1
    echo "cat" >> profiles/categories || exit 1
    cat <<END > profiles/profile/virtuals
END
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
    mkdir -p "cat/simple"
    cat <<END > cat/simple/simple-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
SRC_URI=""
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"
S="\${WORKDIR}"

src_unpack() {
    touch installed-${e}
}

src_install() {
    insinto /usr/share
    doins installed-${e}
}
END

    cd ..

    mkdir -p binrepo${e}/{profiles/profile,metadata,eclass} || exit 1
    cd binrepo${e} || exit 1
    echo "binrepo${e}" >> profiles/repo_name || exit 1
    echo > profiles/categories || exit 1
    cat <<END > profiles/profile/virtuals
END
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

    cd ..
done

for e in exheres-0 ; do
    mkdir -p repo${e}/{profiles/profile,metadata,eclass} || exit 1
    cd repo${e} || exit 1
    echo "repo${e}" >> profiles/repo_name || exit 1
    echo "cat" >> profiles/categories || exit 1
    cat <<END > profiles/profile/virtuals
END
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

    mkdir -p "cat/simple"
    cat <<END > cat/simple/simple-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="\${WORKBASE}"

src_unpack() {
    touch installed-${e}
}

src_install() {
    insinto /usr/share
    doins installed-${e}
}
END

    mkdir -p "cat/symlinks"
    cat <<END > cat/symlinks/symlinks-1.ebuild || exit 1
EAPI="${e}"
DESCRIPTION="The Description"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS=""
LICENCES="GPL-2"
PLATFORMS="test"
WORK="\${WORKBASE}"

src_unpack() {
    touch symlinks-b
}

src_install() {
    insinto /usr/share
    dosym symlinks-b /usr/share/symlinks-a
    doins symlinks-b
    dosym /usr/share/symlinks-b /usr/share/symlinks-c
    find \${IMAGE} | xargs ls -ld
}
END

    cd ..

    mkdir -p binrepo${e}/{profiles/profile,metadata,eclass} || exit 1
    cd binrepo${e} || exit 1
    echo "binrepo${e}" >> profiles/repo_name || exit 1
    echo > profiles/categories || exit 1
    cat <<END > profiles/profile/virtuals
END
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

    cd ..
done

cd ..

