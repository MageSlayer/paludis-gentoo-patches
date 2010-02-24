#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_blockers_dir || exit 1
cd resolver_TEST_blockers_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# hard
echo 'hard' >> metadata/categories.conf

mkdir -p 'packages/hard/target'
cat <<END > packages/hard/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !hard/a-pkg[<2] !hard/z-pkg[<2] )
    "
END

mkdir -p 'packages/hard/a-pkg'
cat <<END > packages/hard/a-pkg/a-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/hard/z-pkg'
cat <<END > packages/hard/z-pkg/z-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# unfixable
echo 'unfixable' >> metadata/categories.conf

mkdir -p 'packages/unfixable/target'
cat <<END > packages/unfixable/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !unfixable/a-pkg )
    "
END

mkdir -p 'packages/unfixable/a-pkg'
cat <<END > packages/unfixable/a-pkg/a-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# remove
echo 'remove' >> metadata/categories.conf

mkdir -p 'packages/remove/target'
cat <<END > packages/remove/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !remove/a-pkg )
    "
END

mkdir -p 'packages/remove/a-pkg'
cat <<END > packages/remove/a-pkg/a-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# blocked-and-dep
echo 'blocked-and-dep' >> metadata/categories.conf

mkdir -p 'packages/blocked-and-dep/target'
cat <<END > packages/blocked-and-dep/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !blocked-and-dep/both blocked-and-dep/both )
    "
END

mkdir -p 'packages/blocked-and-dep/both'
cat <<END > packages/blocked-and-dep/both/both-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..

