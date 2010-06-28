#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_fetch_dir || exit 1
cd resolver_TEST_fetch_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# fetch
echo 'fetch' >> metadata/categories.conf

mkdir -p 'packages/fetch/target'
cat <<END > packages/fetch/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    fetch: fetch/fetch-dep
    "
END

mkdir -p 'packages/fetch/fetch-dep'
cat <<END > packages/fetch/fetch-dep/fetch-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..

