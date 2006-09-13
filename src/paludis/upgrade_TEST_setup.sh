#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir upgrade_TEST_dir || exit 1
cd upgrade_TEST_dir || exit 1

mkdir -p config/.paludis-upgrade-test
cat <<END > config/.paludis-upgrade-test/specpath
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
profiles = \${location}/profiles/testprofile
buildroot = `pwd`/build
END

cat <<END > root/${SYSCONFDIR}/paludis/repositories/installed.conf
location = \${ROOT}/var/db/pkg
format = vdb
buildroot = `pwd`/build
END

mkdir -p root/tmp
touch root/${SYSCONFDIR}/ld.so.conf
mkdir -p root/var/db/pkg

mkdir -p repo1/{eclass,distfiles,profiles/testprofile,test-category/target/files} || exit 1

mkdir -p src/target-1
cat <<"END" > src/target-1/testbin
#!/bin/bash
echo "This is testbin-1"
END
chmod +x src/target-1/testbin
cat <<"END" > src/target-1/testbin1
#!/bin/bash
echo "This is testbin1"
END
chmod +x src/target-1/testbin1
cd src
tar zcf target-1.tar.gz target-1/
mv target-1.tar.gz ../repo1/distfiles/
cd ..
rm -fr src

mkdir -p src/target-2
cat <<"END" > src/target-2/testbin
#!/bin/bash
echo "This is testbin-2"
END
chmod +x src/target-2/testbin
cat <<"END" > src/target-2/testbin2
#!/bin/bash
echo "This is testbin2"
END
chmod +x src/target-2/testbin2
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

cat <<"END" > eclass/myeclass.eclass || exit 1
the_eclass_works()
{
    true
}
END

cat <<"END" > test-category/target/target-1.ebuild || exit 1
inherit myeclass

DESCRIPTION="Test target"
HOMEPAGE="http://paludis.berlios.de/"
SRC_URI="http://invalid.domain/${P}.tar.gz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

pkg_setup() {
    export VAR1=yes
    VAR2=yes
    local VAR3=yes
}

src_install() {
    [[ "${VAR1}" == yes ]] || die
    [[ "${VAR2}" == yes ]] || die
    [[ "${VAR3}" == yes ]] && die

    dobin testbin
    dobin testbin${PV}
}

pkg_prerm() {
    [[ "${VAR1}" == yes ]] || die
    [[ "${VAR2}" == yes ]] || die
    [[ "${VAR3}" == yes ]] && die

    the_eclass_works || die
}
END

cat <<"END" > test-category/target/target-2.ebuild || exit 1
DESCRIPTION="Test target"
HOMEPAGE="http://paludis.berlios.de/"
SRC_URI="http://invalid.domain/${P}.tar.gz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin testbin
    dobin testbin${PV}
}
END
cd ..

