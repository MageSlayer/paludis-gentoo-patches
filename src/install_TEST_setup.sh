#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir install_TEST_dir || exit 1
cd install_TEST_dir || exit 1

mkdir -p config/.paludis-install-test
cat <<END > config/.paludis-install-test/specpath
root = `pwd`/root
config-suffix =
END

mkdir -p root/${SYSCONFDIR}/paludis/repositories
cat <<END > root/${SYSCONFDIR}/paludis/use.conf
* foo
END

cat <<END > root/${SYSCONFDIR}/paludis/licenses.conf
* *
END

cat <<END > root/${SYSCONFDIR}/paludis/keywords.conf
* test
END

cat <<END > root/${SYSCONFDIR}/paludis/bashrc
export CHOST="my-chost"
export USER_BASHRC_WAS_USED=yes
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/repo1.conf
location = `pwd`/repo1
cache = /var/empty
format = portage
profile = \${location}/profiles/testprofile
buildroot = `pwd`/build
END

mkdir -p root/tmp
touch root/${SYSCONFDIR}/ld.so.conf

mkdir -p repo1/{eclass,distfiles,profiles/testprofile,test-category/target/files} || exit 1

mkdir -p src/target-2
cat <<"END" > src/target-2/testbin
#!/bin/bash
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
END

cat <<"END" > test-category/target/target-2.ebuild || exit 1
DESCRIPTION="Test target"
HOMEPAGE="http://paludis.berlios.de/"
SRC_URI="http://invalid.domain/${P}.tar.gz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    [[ -z "${USER_BASHRC_WAS_USED}" ]] && die "bad env"
}

src_test() {
    ./testbin | grep success || die "failure"
}

src_install() {
    dobin testbin
}
END
cd ..

