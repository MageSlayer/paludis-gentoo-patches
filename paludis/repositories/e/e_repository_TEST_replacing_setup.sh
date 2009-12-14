#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_replacing_dir || exit 1
cd e_repository_TEST_replacing_dir || exit 1

mkdir -p root/etc

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

mkdir -p repo1/{profiles/profile,metadata,eclass} || exit 1
cd repo1 || exit 1
echo "test-repo-1" >> profiles/repo_name || exit 1
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
END
mkdir -p "packages/cat/replace-none"
cat <<'END' > packages/cat/replace-none/replace-none-1.exheres-0 || exit 1
WORK="${WORKBASE}"
PLATFORMS="test"
FOO=true
SLOT="0"

pkg_setup() {
    [[ -z "${REPLACING_IDS}" ]] || die "REPLACING_IDS is ${REPLACING_IDS}"
}
END
mkdir -p "packages/cat/replace-one"
cat <<'END' > packages/cat/replace-one/replace-one-1.exheres-0 || exit 1
WORK="${WORKBASE}"
PLATFORMS="test"
FOO=true
SLOT="0"

pkg_setup() {
    [[ "${REPLACING_IDS}" == "cat/pkg-1:1::installed" ]] || die "REPLACING_IDS is ${REPLACING_IDS}"
}
END
mkdir -p "packages/cat/replace-many"
cat <<'END' > packages/cat/replace-many/replace-many-1.exheres-0 || exit 1
WORK="${WORKBASE}"
PLATFORMS="test"
FOO=true
SLOT="0"

pkg_setup() {
    [[ "${REPLACING_IDS}" == \
        "cat/pkg-1:1::installed cat/pkg-2:2::installed cat/pkg-3:3::installed" ]] \
        || die "REPLACING_IDS is ${REPLACING_IDS}"
}
END
cd ..

mkdir -p repo2/{profiles/profile,metadata,eclass} || exit 1
cd repo2 || exit 1
echo "test-repo-1" >> profiles/repo_name || exit 1
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
ARCH="dead-badger"
KERNEL="linux"
END
mkdir -p "packages/cat/replace-none"
cat <<'END' > packages/cat/replace-none/replace-none-1.ebuild || exit 1
S="${WORKDIR}"
KEYWORDS="test"
SLOT="0"
EAPI="4"

pkg_setup() {
    [[ -z "${REPLACING_VERSIONS}" ]] || die "REPLACING_VERSIONS is ${REPLACING_VERSIONS}"
}
END
mkdir -p "packages/cat/replace-one"
cat <<'END' > packages/cat/replace-one/replace-one-1.ebuild || exit 1
S="${WORKDIR}"
KEYWORDS="test"
SLOT="0"
EAPI="4"

pkg_setup() {
    [[ "${REPLACING_VERSIONS}" == "1" ]] || die "REPLACING_VERSIONS is ${REPLACING_VERSIONS}"
}
END
mkdir -p "packages/cat/replace-many"
cat <<'END' > packages/cat/replace-many/replace-many-1.ebuild || exit 1
S="${WORKDIR}"
KEYWORDS="test"
SLOT="0"
EAPI="4"

pkg_setup() {
    [[ "${REPLACING_VERSIONS}" == "1 2 3" ]] || die "REPLACING_VERSIONS is ${REPLACING_VERSIONS}"
}
END
cd ..
cd ..

