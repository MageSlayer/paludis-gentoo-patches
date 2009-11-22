#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_cycles_dir || exit 1
cd resolver_TEST_cycles_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# no-changes
echo 'no-changes' >> metadata/categories.conf

mkdir -p 'packages/no-changes/target'
cat <<END > packages/no-changes/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="no-changes/dep-a no-changes/dep-b"
END

mkdir -p 'packages/no-changes/dep-a'
cat <<END > packages/no-changes/dep-a/dep-a-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="no-changes/dep-b"
END

mkdir -p 'packages/no-changes/dep-b'
cat <<END > packages/no-changes/dep-b/dep-b-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="no-changes/dep-a"
END

# existing-usable
echo 'existing-usable' >> metadata/categories.conf

mkdir -p 'packages/existing-usable/target'
cat <<END > packages/existing-usable/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="existing-usable/dep"
END

mkdir -p 'packages/existing-usable/dep'
cat <<END > packages/existing-usable/dep/dep-2.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="existing-usable/target"
END

cd ..

