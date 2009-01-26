#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_ever_dir || exit 1
cd e_repository_TEST_ever_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

mkdir -p repo1/{profiles/profile,metadata,eclass} || exit 1
cd repo1 || exit 1
echo "test-repo-1" >> profiles/repo_name || exit 1
cat <<END > profiles/profile/virtuals
virtual/virtual-pretend-installed cat/pretend-installed
virtual/virtual-doesnotexist cat/doesnotexist
END
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
END
mkdir -p "packages/cat/ever-at-least"
cat <<'END' > packages/cat/ever-at-least/ever-at-least-1.3.ebuild || exit 1
if ever at_least 2 ; then
    DESCRIPTION="Really Not The Long Description"
    SUMMARY="Really Not The Short Description"
elif ever at_least 1.2.3 ; then
    DESCRIPTION="The Long Description"
    SUMMARY="The Short Description"
else
    DESCRIPTION="Not The Long Description"
    SUMMARY="Not The Short Description"
fi

HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

pkg_setup() {
    ever at_least 1.2 || die "at_least 1.2"
    ever at_least 1.3 || die "at_least 1.3"
    ever at_least 1.4 && die "at_least 1.4"

    ever at_least 1.2 1.2 || die "at_least 1.2 1.2"
    ever at_least 1.3 1.2 && die "at_least 1.3 1.2"
    ever at_least 1.4 1.2 && die "at_least 1.4 1.2"
}
END
mkdir -p "packages/cat/ever-split"
cat <<'END' > packages/cat/ever-split/ever-split-1.ebuild || exit 1
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

check() {
    a=$(ever split $1 )
    [[ "${a}" == "${2}" ]] || die "got $a wanted $2"
}

pkg_setup() {
    check 0.8.3 "0 8 3"
    check 7c "7 c"
    check 3.0_p2 "3 0 p2"
    check 20040905 "20040905"
    check 3.0c-r1 "3 0 c r1"
}
END
mkdir -p "packages/cat/ever-split-all"
cat <<'END' > packages/cat/ever-split-all/ever-split-all-1.ebuild || exit 1
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

check() {
    a=$(ever split_all $1 )
    [[ "${a}" == "${2}" ]] || die "got $a wanted $2"
}

pkg_setup() {
    check 0.8.3 "0 . 8 . 3"
    check 7c "7 c"
    check 3.0_p2 "3 . 0 _ p2"
    check 20040905 "20040905"
    check 3.0c-r1 "3 . 0 c - r1"
}
END
mkdir -p "packages/cat/ever-major"
cat <<'END' > packages/cat/ever-major/ever-major-1.ebuild || exit 1
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

check() {
    a=$(ever major $1 )
    [[ "${a}" == "${2}" ]] || die "got $a wanted $2"
}

pkg_setup() {
    check 0.8.3 "0"
    check 7c "7"
    check 3.0_p2 "3"
    check 20040905 "20040905"
    check 3.0c-r1 "3"
    check scm "scm"
    check "" "1"
}
END
mkdir -p "packages/cat/ever-range"
cat <<'END' > packages/cat/ever-range/ever-range-1.ebuild || exit 1
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

check() {
    a=$(ever range $1 $2 )
    [[ "${a}" == "${3}" ]] || die "got $a wanted $3"
}

pkg_setup() {
    check "1" "1.2.3" "1"
    check "1-2" "1.2.3" "1.2"
    check "2-" "1.2.3" "2.3"
    check "-2" "1.2.3" "1.2"
    check "1" "" "1"
}
END
cd ..

cd ..

