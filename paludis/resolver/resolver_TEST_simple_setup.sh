#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_simple_dir || exit 1
cd resolver_TEST_simple_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# no-deps
echo 'no-deps' >> metadata/categories.conf

mkdir -p 'packages/no-deps/target'
cat <<END > packages/no-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# build-deps
echo 'build-deps' >> metadata/categories.conf

mkdir -p 'packages/build-deps/target'
cat <<END > packages/build-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build: build-deps/a-dep build-deps/b-dep build-deps/z-dep"
END

mkdir -p 'packages/build-deps/a-dep'
cat <<END > packages/build-deps/a-dep/a-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/build-deps/b-dep'
cat <<END > packages/build-deps/b-dep/b-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/build-deps/z-dep'
cat <<END > packages/build-deps/z-dep/z-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# run-deps
echo 'run-deps' >> metadata/categories.conf

mkdir -p 'packages/run-deps/target'
cat <<END > packages/run-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: run-deps/a-dep run-deps/b-dep run-deps/z-dep"
END

mkdir -p 'packages/run-deps/a-dep'
cat <<END > packages/run-deps/a-dep/a-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/run-deps/b-dep'
cat <<END > packages/run-deps/b-dep/b-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/run-deps/z-dep'
cat <<END > packages/run-deps/z-dep/z-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# post-deps
echo 'post-deps' >> metadata/categories.conf

mkdir -p 'packages/post-deps/target'
cat <<END > packages/post-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="post: post-deps/a-dep post-deps/b-dep post-deps/z-dep"
END

mkdir -p 'packages/post-deps/a-dep'
cat <<END > packages/post-deps/a-dep/a-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/post-deps/b-dep'
cat <<END > packages/post-deps/b-dep/b-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

mkdir -p 'packages/post-deps/z-dep'
cat <<END > packages/post-deps/z-dep/z-dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

cd ..

