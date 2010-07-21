#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir upgrade_TEST_dir || exit 1
cd upgrade_TEST_dir || exit 1
mkdir -p build

mkdir -p vdb_config/.paludis-upgrade-test
cat <<END > vdb_config/.paludis-upgrade-test/specpath.conf
root = `pwd`/root
config-suffix = vdb
END

mkdir -p exndbam_config/.paludis-upgrade-test
cat <<END > exndbam_config/.paludis-upgrade-test/specpath.conf
root = `pwd`/root
config-suffix = exndbam
END

for a in vdb exndbam ; do

    mkdir -p root/${SYSCONFDIR}/paludis-${a}/repositories
    cat <<END > root/${SYSCONFDIR}/paludis-${a}/use.conf
*/* foo
END

    cat <<END > root/${SYSCONFDIR}/paludis-${a}/licenses.conf
*/* *
END

    cat <<END > root/${SYSCONFDIR}/paludis-${a}/keywords.conf
*/* test
END

    cat <<END > root/${SYSCONFDIR}/paludis-${a}/bashrc
export CHOST="my-chost"
export USER_BASHRC_WAS_USED=yes
END

    cat <<END > root/${SYSCONFDIR}/paludis-${a}/repositories/repo1.conf
location = `pwd`/repo1
cache = /var/empty
format = e
names_cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`/build
END

done

cat <<END > root/${SYSCONFDIR}/paludis-vdb/repositories/installed.conf
location = \${ROOT}/var/db/pkg
format = vdb
names_cache = /var/empty
provides_cache = /var/empty
builddir = `pwd`/build
END

cat <<END > root/${SYSCONFDIR}/paludis-exndbam/repositories/installed.conf
location = \${ROOT}/var/db/exndbam
format = exndbam
builddir = `pwd`/build
END

mkdir -p root/tmp
touch root/${SYSCONFDIR}/ld.so.conf
mkdir -p root/var/db/pkg

mkdir -p root/var/db/exndbam

mkdir -p repo1/{eclass,distfiles,profiles/testprofile,test-category/target/files} || exit 1

mkdir -p src/target-1
cat <<"END" > src/target-1/testbin
#!/usr/bin/env bash
echo "This is testbin-1"
END
chmod +x src/target-1/testbin
cat <<"END" > src/target-1/testbin1
#!/usr/bin/env bash
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
#!/usr/bin/env bash
echo "This is testbin-2"
END
chmod +x src/target-2/testbin
cat <<"END" > src/target-2/testbin2
#!/usr/bin/env bash
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
EAPI="3"

inherit myeclass

DESCRIPTION="Test target"
HOMEPAGE="http://paludis.pioto.org/"
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

src_compile() {
    if [[ ${REPEAT} == yes ]] ; then
        sed -i -e 's,testbin1,repeatbin1,' testbin1 || die
    fi
}

src_install() {
    [[ "${VAR1}" == yes ]] || die
    [[ "${VAR2}" == yes ]] || die
    [[ "${VAR3}" == yes ]] && die
    [[ $(printenv VAR1 ) == yes ]] || die

    dobin testbin
    dobin testbin${PV}

    touch -d "1 April 2000" -m ${D}/usr/bin/testbin
}

pkg_preinst() {
    [[ "${VAR1}" == yes ]] || die
    [[ "${VAR2}" == yes ]] || die
    [[ "${VAR3}" == yes ]] && die
    [[ $(printenv VAR1 ) == yes ]] || die
    [[ -z "$(printenv D )" ]] && die

    the_eclass_works || die
}

pkg_prerm() {
    [[ "${VAR1}" == yes ]] || die
    [[ "${VAR2}" == yes ]] || die
    [[ "${VAR3}" == yes ]] && die
    [[ $(printenv VAR1 ) == yes ]] || die
    [[ -z "$(printenv D )" ]] && die

    the_eclass_works || die
}
END

cat <<"END" > test-category/target/target-2.ebuild || exit 1
EAPI="3"

DESCRIPTION="Test target"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://invalid.domain/${P}.tar.gz"
SLOT="0"
IUSE=""
LICENSE="GPL-2"
KEYWORDS="test"

src_install() {
    dobin testbin
    dobin testbin${PV}

    touch -d "1 April 2000" -m ${D}/usr/bin/testbin
}
END
cd ..

