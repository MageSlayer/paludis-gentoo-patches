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

# mutual-run-deps
echo 'mutual-run-deps' >> metadata/categories.conf

mkdir -p 'packages/mutual-run-deps/target'
cat <<END > packages/mutual-run-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="mutual-run-deps/dep-a"
END

mkdir -p 'packages/mutual-run-deps/dep-a'
cat <<END > packages/mutual-run-deps/dep-a/dep-a-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: mutual-run-deps/dep-b mutual-run-deps/dep-c"
END

mkdir -p 'packages/mutual-run-deps/dep-b'
cat <<END > packages/mutual-run-deps/dep-b/dep-b-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: mutual-run-deps/dep-a"
END

mkdir -p 'packages/mutual-run-deps/dep-c'
cat <<END > packages/mutual-run-deps/dep-c/dep-c-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: mutual-run-deps/dep-b"
END

# mutual-build-deps
echo 'mutual-build-deps' >> metadata/categories.conf

mkdir -p 'packages/mutual-build-deps/target'
cat <<END > packages/mutual-build-deps/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="mutual-build-deps/dep-a"
END

mkdir -p 'packages/mutual-build-deps/dep-a'
cat <<END > packages/mutual-build-deps/dep-a/dep-a-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build: mutual-build-deps/dep-b mutual-build-deps/dep-c"
END

mkdir -p 'packages/mutual-build-deps/dep-b'
cat <<END > packages/mutual-build-deps/dep-b/dep-b-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build: mutual-build-deps/dep-a"
END

mkdir -p 'packages/mutual-build-deps/dep-c'
cat <<END > packages/mutual-build-deps/dep-c/dep-c-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="build: mutual-build-deps/dep-b"
END

# triangle
echo 'triangle' >> metadata/categories.conf

mkdir -p 'packages/triangle/target'
cat <<END > packages/triangle/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="triangle/dep-c"
END

mkdir -p 'packages/triangle/dep-a'
cat <<END > packages/triangle/dep-a/dep-a-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: triangle/dep-b build: triangle/dep-c"
END

mkdir -p 'packages/triangle/dep-b'
cat <<END > packages/triangle/dep-b/dep-b-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: triangle/dep-a build: triangle/dep-c"
END

mkdir -p 'packages/triangle/dep-c'
cat <<END > packages/triangle/dep-c/dep-c-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="run: triangle/dep-b"
END

# self
for r in "build" "run" ; do
    cat=self-${r}
    echo $cat >> metadata/categories.conf

    mkdir -p 'packages/'$cat'/target'
    cat <<END > packages/$cat/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="${cat}/dep"
END

    mkdir -p 'packages/'$cat'/dep'
    cat <<END > packages/$cat/dep/dep-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="${r}: ${cat}/dep[=1]"
END

    done

cd ..

