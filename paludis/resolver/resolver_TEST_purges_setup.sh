#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_purges_dir || exit 1
cd resolver_TEST_purges_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# purges
echo 'purges' >> metadata/categories.conf

mkdir -p 'packages/purges/target'
cat <<END > packages/purges/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    purges/still-used-dep
    purges/new-dep
    "
END

mkdir -p 'packages/purges/new-dep'
cat <<END > packages/purges/new-dep/new-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/purges/still-used-dep'
cat <<END > packages/purges/still-used-dep/still-used-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..

