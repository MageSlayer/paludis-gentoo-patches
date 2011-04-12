#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p vdb_repository_TEST_dir || exit 1
cd vdb_repository_TEST_dir || exit 1

mkdir -p distdir
mkdir -p build
mkdir -p root/etc

mkdir -p repo1/cat-{one/{pkg-one-1,pkg-both-1},two/{pkg-two-2,pkg-both-2}} || exit 1

for i in SLOT EAPI; do
    echo "0" >repo1/cat-one/pkg-one-1/${i}
done

for i in DEPEND RDEPEND LICENSE INHERITED IUSE PDEPEND PROVIDE; do
    touch repo1/cat-one/pkg-one-1/${i}
done

echo "test flag1 flag2 kernel_linux" >>repo1/cat-one/pkg-one-1/USE
echo "flag1 flag2 flag3" >>repo1/cat-one/pkg-one-1/IUSE
echo "KERNEL" >repo1/cat-one/pkg-one-1/USE_EXPAND

cat <<END >repo1/cat-one/pkg-one-1/CONTENTS
dir /directory
  obj /directory/file 4 2
sym /directory/symlink -> target 2 
dir	/directory with spaces
dir /directory with trailing space 
dir /directory  with  consecutive  spaces
obj /file with spaces 4    2
obj /file  with  consecutive  spaces 4 	  2 	  
obj /file with  trailing   space	  4 2
sym /symlink -> target  with  consecutive  spaces 2
sym /symlink with spaces -> target with spaces 2
sym /symlink  with  consecutive  spaces -> target  with  consecutive  spaces 2
sym /symlink -> target -> with -> multiple -> arrows 2
sym /symlink -> target with trailing space  2
sym /symlink ->  target with leading space 2
sym /symlink with trailing space  -> target 2
fif /fifo
fif /fifo with spaces
fif /fifo  with  consecutive  spaces
dev /device
dev /device with spaces
dev /device  with  consecutive  spaces
misc /miscellaneous
misc /miscellaneous with spaces
misc /miscellaneous  with  consecutive  spaces

obj 
  obj	
obj /file
obj /file   
obj /file  2 
sym foobar 2
sym foo -> bar
sym -> bar 2
sym foo -> 2
END

touch "world-empty"
cat <<END > world-no-match
cat-one/foo
cat-one/bar
cat-one/oink
END
cat <<END > world-match
cat-one/foo
cat-one/foofoo
cat-one/bar
END
cat <<END > world-no-match-no-eol
cat-one/foo
cat-one/bar
END
echo -n "cat-one/oink" >> world-no-match-no-eol

mkdir -p repo2/category/package-1 || exit 1
echo "exheres-0" >repo2/category/package-1/EAPI
echo "0" >repo2/category/package-1/SLOT
echo "cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: cat/pkg4 post: cat/pkg5" >repo2/category/package-1/DEPENDENCIES

mkdir -p reinstalltest reinstalltest_src{1,2}/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > reinstalltest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo reinstalltest_src1 >reinstalltest_src1/profiles/repo_name
echo reinstalltest_src2 >reinstalltest_src2/profiles/repo_name
echo cat >reinstalltest_src1/profiles/categories
echo cat >reinstalltest_src2/profiles/categories

cat <<END >reinstalltest_src1/cat/pkg/pkg-1.ebuild
KEYWORDS="test"
SLOT="0"
END
cp reinstalltest_src1/cat/pkg/pkg-1.ebuild reinstalltest_src2/cat/pkg/pkg-1-r0.ebuild

mkdir -p postinsttest postinsttest_src1/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > postinsttest_src1/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo postinsttest >postinsttest_src1/profiles/repo_name
echo cat >postinsttest_src1/profiles/categories

cat <<END >postinsttest_src1/cat/pkg/pkg-0.ebuild
if [[ \${PV} == 0* ]]; then
    EAPI=1
else
    EAPI=paludis-1
fi
KEYWORDS="test"
if [[ \${PV} == 2* ]]; then
    SLOT="2"
else
    SLOT="1"
fi
pkg_preinst() {
    OTHER=\$(best_version "\${CATEGORY}/\${PN}:\${SLOT}")
    if [[ -n \${OTHER} ]]; then
        if [[ \${EAPI} == paludis-1 ]] || has_version "=\${CATEGORY}/\${PF}:\${SLOT}"; then
            COMMAND=rmdir
        else
            COMMAND=mkdir
        fi
    else
        COMMAND=:
    fi
}
pkg_postinst() {
    \${COMMAND} "\${ROOT}"/\${OTHER##*/} || die
}
pkg_postrm() {
    if [[ \$VDB_REPOSITORY_TEST_RMDIR == \${PV} ]] ; then
        rmdir "\${ROOT}"/\${PF} || die "rmdir failed, pv is \${PV}, comparing to \$VDB_REPOSITORY_TEST_RMDIR"
    else
        mkdir "\${ROOT}"/\${PF} || die "mkdir failed, pv is \${PV}, comparing to \$VDB_REPOSITORY_TEST_RMDIR"
    fi
}
END
cp postinsttest_src1/cat/pkg/pkg-{0,0.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,1.1}.ebuild
cp postinsttest_src1/cat/pkg/pkg-{0,2}.ebuild

mkdir -p removestalefilesvdb removestalefiles/{eclass,profiles/profile,cat/pkg} || exit 1

cat <<END > removestalefiles/profiles/profile/make.defaults
ARCH=test
USERLAND="GNU"
KERNEL="linux"
CHOST="i286-badger-linux-gnu"
END
echo removestalefiles >removestalefiles/profiles/repo_name
echo cat >removestalefiles/profiles/categories

cat <<'END' >removestalefiles/cat/pkg/pkg-0.ebuild
SLOT="0"
KEYWORDS="test"
S="${WORKDIR}"

src_unpack() {
    touch stale-both
    $VDB_REPOSITORY_TEST_STALE && touch stale-first
}

src_install() {
    insinto /
    doins stale-*
}
END

