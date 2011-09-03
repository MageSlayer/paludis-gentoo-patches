#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_binaries_dir || exit 1
cd resolver_TEST_binaries_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p binrepo/{profiles/profile,metadata}

cd binrepo
echo "binrepo" > profiles/repo_name
: > metadata/categories.conf

cd ..

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# self-build-binary
echo 'self-build-binary' >> metadata/categories.conf

mkdir -p 'packages/self-build-binary/target'
cat <<END > packages/self-build-binary/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build+run: self-build-binary/dep"
END

mkdir -p 'packages/self-build-binary/dep'
cat <<END > packages/self-build-binary/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build: self-build-binary/dep"
END

# self-run-binary
echo 'self-run-binary' >> metadata/categories.conf

mkdir -p 'packages/self-run-binary/target'
cat <<END > packages/self-run-binary/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build+run: self-run-binary/dep"
END

mkdir -p 'packages/self-run-binary/dep'
cat <<END > packages/self-run-binary/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: self-run-binary/dep"
END

cd ..

