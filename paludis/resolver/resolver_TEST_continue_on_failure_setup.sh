#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_continue_on_failure_dir || exit 1
cd resolver_TEST_continue_on_failure_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# continue-on-failure
echo 'continue-on-failure' >> metadata/categories.conf

mkdir -p 'packages/continue-on-failure/target'
cat <<END > packages/continue-on-failure/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    continue-on-failure/unchanged-dep
    continue-on-failure/direct-dep
    "
END

mkdir -p 'packages/continue-on-failure/direct-dep'
cat <<END > packages/continue-on-failure/direct-dep/direct-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/continue-on-failure/unchanged-dep'
cat <<END > packages/continue-on-failure/unchanged-dep/unchanged-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    continue-on-failure/indirect-dep
    "
END

mkdir -p 'packages/continue-on-failure/indirect-dep'
cat <<END > packages/continue-on-failure/indirect-dep/indirect-dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# continue-on-failure-purge
echo 'continue-on-failure-purge' >> metadata/categories.conf

mkdir -p 'packages/continue-on-failure-purge/target'
cat <<END > packages/continue-on-failure-purge/target/target-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..

