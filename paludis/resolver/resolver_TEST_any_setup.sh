#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_any_dir || exit 1
cd resolver_TEST_any_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# test
echo 'test' >> metadata/categories.conf

mkdir -p 'packages/test/target'
cat <<END > packages/test/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( test/dep[>=2] ( ) )
    "
END

mkdir -p 'packages/test/dep'
cat <<END > packages/test/dep/dep-3.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# preferences
echo 'preferences' >> metadata/categories.conf

mkdir -p 'packages/preferences/target'
cat <<END > packages/preferences/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( preferences/dep-a preferences/dep-middle preferences/dep-b )
    "
END

mkdir -p 'packages/preferences/dep-a'
cat <<END > packages/preferences/dep-a/dep-a-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/preferences/dep-middle'
cat <<END > packages/preferences/dep-middle/dep-middle-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/preferences/dep-b'
cat <<END > packages/preferences/dep-b/dep-b-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..

