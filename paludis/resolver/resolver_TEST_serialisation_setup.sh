#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_serialisation_dir || exit 1
cd resolver_TEST_serialisation_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# serialisation
echo 'serialisation' >> metadata/categories.conf

mkdir -p 'packages/serialisation/target'
cat <<END > packages/serialisation/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( build: serialisation/dep serialisation/error )
    ( suggestion: serialisation/suggestion )
    "
END

mkdir -p 'packages/serialisation/dep'
cat <<END > packages/serialisation/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/serialisation/suggestion'
cat <<END > packages/serialisation/suggestion/suggestion-1.exheres-0
SUMMARY="suggestion"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/serialisation/error'
cat <<END > packages/serialisation/error/error-1.exheres-0
SUMMARY="error"
SLOT="0"
END

cd ..


