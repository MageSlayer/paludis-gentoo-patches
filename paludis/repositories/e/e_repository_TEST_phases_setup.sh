#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_phases_dir || exit 1
cd e_repository_TEST_phases_dir || exit 1

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
OPTIONS="weasel spinach"
END
mkdir -p "packages/cat/no-expensive-test"
cat <<'END' > packages/cat/no-expensive-test/no-expensive-test-1.0.exheres-0 || exit 1
DESCRIPTION="foo"
SUMMARY="foo"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"
END
mkdir -p "packages/cat/expensive-test"
cat <<'END' > packages/cat/expensive-test/expensive-test-1.0.exheres-0 || exit 1
DESCRIPTION="foo"
SUMMARY="foo"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

src_test_expensive() {
    true
}
END
mkdir -p "packages/cat/expensive-test-fail"
cat <<'END' > packages/cat/expensive-test-fail/expensive-test-fail-1.0.exheres-0 || exit 1
DESCRIPTION="foo"
SUMMARY="foo"
HOMEPAGE="http://example.com/"
DOWNLOADS=""
SLOT="0"
MYOPTIONS="spork"
LICENCES="GPL-2"
PLATFORMS="test"

src_test_expensive() {
    die
}
END
cd ..

