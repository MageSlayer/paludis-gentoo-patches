#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_suggestions_dir || exit 1
cd resolver_TEST_suggestions_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# suggestion-then-dependency
echo 'suggestion-then-dependency' > metadata/categories.conf

mkdir -p 'packages/suggestion-then-dependency/target'
cat <<END > packages/suggestion-then-dependency/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( build,required: suggestion-then-dependency/hard-dep )
    ( post,suggested: suggestion-then-dependency/a-suggested-dep )
    "
END

mkdir -p 'packages/suggestion-then-dependency/hard-dep'
cat <<END > packages/suggestion-then-dependency/hard-dep/hard-dep-1.exheres-0
SUMMARY="hard dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    build,required: suggestion-then-dependency/a-suggested-dep
    "
END

mkdir -p 'packages/suggestion-then-dependency/a-suggested-dep'
cat <<END > packages/suggestion-then-dependency/a-suggested-dep/a-suggested-dep-1.exheres-0
SUMMARY="suggested dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

cd ..

