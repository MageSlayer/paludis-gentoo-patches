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

# suggestion
echo 'suggestion' >> metadata/categories.conf

mkdir -p 'packages/suggestion/target'
cat <<END > packages/suggestion/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( suggestion: suggestion/dep )
    "
END

mkdir -p 'packages/suggestion/dep'
cat <<END > packages/suggestion/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# unmeetable-suggestion
echo 'unmeetable-suggestion' >> metadata/categories.conf

mkdir -p 'packages/unmeetable-suggestion/target'
cat <<END > packages/unmeetable-suggestion/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( suggestion: unmeetable-suggestion/unmeetable-dep )
    "
END

mkdir -p 'packages/unmeetable-suggestion/unmeetable-dep'
cat <<END > packages/unmeetable-suggestion/unmeetable-dep/unmeetable-dep-1.exheres-0
SUMMARY="unmeetable dep"
PLATFORMS=""
SLOT="0"
END

# suggestion-then-dependency
echo 'suggestion-then-dependency' >> metadata/categories.conf

mkdir -p 'packages/suggestion-then-dependency/target'
cat <<END > packages/suggestion-then-dependency/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( build: suggestion-then-dependency/hard-dep )
    ( suggestion: suggestion-then-dependency/a-suggested-dep )
    "
END

mkdir -p 'packages/suggestion-then-dependency/hard-dep'
cat <<END > packages/suggestion-then-dependency/hard-dep/hard-dep-1.exheres-0
SUMMARY="hard dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    build: suggestion-then-dependency/a-suggested-dep
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

