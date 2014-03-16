#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_promote_binaries_dir || exit 1
cd resolver_TEST_promote_binaries_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p binrepo/{profiles/profile,metadata,packages}

cd binrepo
echo "binrepo" > profiles/repo_name
: > metadata/categories.conf
cd ..

mkdir -p repo/{profiles/profile,metadata,packages}

cd repo
echo "repo" > profiles/repo_name
echo "cat" > metadata/categories.conf

mkdir -p packages/cat/pkg1
cat <<END > packages/cat/pkg1/pkg1-1.exheres-0
SUMMARY="pkg1"
PLATFORMS="test"
MYOPTIONS="opt"
SLOT="0"
END

mkdir -p packages/cat/pkg2
cat <<END > packages/cat/pkg2/pkg2-1.exheres-0
SUMMARY="pkg1"
PLATFORMS="test"
MYOPTIONS="opt"
SLOT="0"
END

cd ..

