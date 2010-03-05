#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_errors_dir || exit 1
cd resolver_TEST_errors_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# unable-to-decide-then-more
echo 'unable-to-decide-then-more' >> metadata/categories.conf

mkdir -p 'packages/unable-to-decide-then-more/target'
cat <<END > packages/unable-to-decide-then-more/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    unable-to-decide-then-more/pkg-a
    unable-to-decide-then-more/pkg-b"
END

mkdir -p 'packages/unable-to-decide-then-more/pkg-b'
cat <<END > packages/unable-to-decide-then-more/pkg-b/pkg-b-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    unable-to-decide-then-more/pkg-a"
END

cd ..

