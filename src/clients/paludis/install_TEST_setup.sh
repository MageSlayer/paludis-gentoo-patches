#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir install_TEST_dir || exit 1
cd install_TEST_dir || exit 1
mkdir -p build

mkdir -p config-checks-always/.paludis-install-test-vdb_config
cat <<END > config-checks-always/.paludis-install-test-vdb_config/specpath.conf
root = `pwd`/root
config-suffix = vdb-checks-always
END

mkdir -p config-checks-always/.paludis-install-test-exndbam_config
cat <<END > config-checks-always/.paludis-install-test-exndbam_config/specpath.conf
root = `pwd`/root
config-suffix = exndbam-checks-always
END

mkdir -p config-checks-none/.paludis-install-test-vdb_config
cat <<END > config-checks-none/.paludis-install-test-vdb_config/specpath.conf
root = `pwd`/root
config-suffix = vdb-checks-none
END

mkdir -p config-checks-none/.paludis-install-test-exndbam_config
cat <<END > config-checks-none/.paludis-install-test-exndbam_config/specpath.conf
root = `pwd`/root
config-suffix = exndbam-checks-none
END

for c in vdb exndbam ; do
    mkdir -p root/${SYSCONFDIR}/paludis-${c}-checks-{always,none}/repositories
    cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-always/use.conf
*/* foo build_options: optional_tests recommended_tests
END

    cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-none/use.conf
*/* foo build_options: -optional_tests -recommended_tests
END

    for d in always none ; do

        cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-${d}/general.conf
world = `pwd`/root/world-${c}
END

        cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-${d}/licenses.conf
*/* *
END

        cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-${d}/keywords.conf
*/* test
END

        cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-${d}/bashrc
export CHOST="my-chost"
export USER_BASHRC_WAS_USED=yes
END

        cat <<END > root/${SYSCONFDIR}/paludis-${c}-checks-${d}/repositories/repo1.conf
location = `pwd`/repo1
cache = /var/empty
format = ebuild
names_cache = /var/empty
profiles = \${location}/profiles/testprofile \${location}/profiles/anothertestprofile
builddir = `pwd`/build
END

    done

done

for d in always none ; do
    cat <<END > root/${SYSCONFDIR}/paludis-vdb-checks-${d}/repositories/installed.conf
location = `pwd`/root/var/db/pkg
format = vdb
names_cache = /var/empty
provides_cache = /var/empty
builddir = `pwd`/build
END

    cat <<END > root/${SYSCONFDIR}/paludis-exndbam-checks-${d}/repositories/installed.conf
location = `pwd`/root/var/db/exndbam
format = exndbam
builddir = `pwd`/build
END

done

mkdir -p root/tmp
mkdir -p root/var/db/pkg
mkdir -p root/var/db/exndbam
touch root/${SYSCONFDIR}/ld.so.conf

mkdir -p repo1/{eclass,distfiles,profiles/{testprofile,anothertestprofile},test-category/target/files} || exit 1

mkdir -p src/target-2
cat <<"END" > src/target-2/testbin
#!/usr/bin/env bash
echo "Test was a success"
END
chmod +x src/target-2/testbin
cd src
tar zcf target-2.tar.gz target-2/
mv target-2.tar.gz ../repo1/distfiles/
cd ..
rm -fr src

cd repo1 || exit 1
echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
test-category
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
TESTPROFILE_WAS_SOURCED=yes
PROFILE_ORDERING=1
USE_EXPAND="USERLAND KERNEL"
END
cat <<END > profiles/anothertestprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
ANOTHERTESTPROFILE_WAS_SOURCED=yes
PROFILE_ORDERING=2
END

cat <<"END" > eclass/foo.eclass
inherit_was_ok() {
    true
}
END

cat <<"END" > test-category/target/target-2.ebuild || exit 1
inherit foo

DESCRIPTION="Test target"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://invalid.domain/${P}.tar.gz oink? ( http://example.com/foo.tar.gz )"
SLOT="0"
IUSE="oink"
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -z "${USER_BASHRC_WAS_USED}" ]] && die "bad env"
    [[ -z "${TESTPROFILE_WAS_SOURCED}" ]] && die "testprofile not sourced"
    [[ -z "${ANOTHERTESTPROFILE_WAS_SOURCED}" ]] && die "anothertestprofile not sourced"
    [[ ${PROFILE_ORDERING:-0} != 2 ]] && die "bad profile source ordering"

    [[ $USERLAND == test ]] || die "bad userland"
    [[ $KERNEL == test ]] || die "bad kernel"
    use userland_test || die "bad use for userland"
    use kernel_test || die "bad use for kernel"
    use test || die "bad use for arch"
}

src_unpack() {
    hasq "${P}.tar.gz" ${A} || die
    hasq "${P}.tar.gz" ${AA} || die
    hasq "foo.tar.gz" ${A} && die
    hasq "foo.tar.gz" ${AA} || die
    unpack ${A}
}

src_compile() {
    inherit_was_ok || die "inherit didn't work"
    ./testbin | grep success || die "failure"
}

src_test() {
    die "giant monkey detected"
}

src_install() {
    dobin testbin
}
END
cd ..

