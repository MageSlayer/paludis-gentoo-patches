#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_any_dir || exit 1
cd resolver_TEST_any_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
:> metadata/categories.conf

# test
echo 'test' >> metadata/categories.conf

mkdir -p 'packages/test/target'
cat <<END > packages/test/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( test/dep[>=2] ( ) )
    "
END

mkdir -p 'packages/test/dep'
cat <<END > packages/test/dep/dep-3.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# preferences
echo 'preferences' >> metadata/categories.conf

mkdir -p 'packages/preferences/target'
cat <<END > packages/preferences/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( preferences/dep-a preferences/dep-middle preferences/dep-b )
    "
END

mkdir -p 'packages/preferences/dep-a'
cat <<END > packages/preferences/dep-a/dep-a-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/preferences/dep-middle'
cat <<END > packages/preferences/dep-middle/dep-middle-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/preferences/dep-b'
cat <<END > packages/preferences/dep-b/dep-b-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

# self-use-first
echo 'self-use-first' >> metadata/categories.conf

mkdir -p 'packages/self-use-first/target'
cat <<END > packages/self-use-first/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    self-use-first/dep
    "
END

mkdir -p 'packages/self-use-first/dep'
cat <<END > packages/self-use-first/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
MYOPTIONS="enabled disabled"
DEPENDENCIES="run: || ( self-use-first/dep[enabled] self-use-first/dep[disabled] )"
END

# self-use-second
echo 'self-use-second' >> metadata/categories.conf

mkdir -p 'packages/self-use-second/target'
cat <<END > packages/self-use-second/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    self-use-second/dep
    "
END

mkdir -p 'packages/self-use-second/dep'
cat <<END > packages/self-use-second/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
MYOPTIONS="enabled disabled"
DEPENDENCIES="run: || ( self-use-second/dep[disabled] self-use-second/dep[enabled] )"
END

# self-use-neither
echo 'self-use-neither' >> metadata/categories.conf

mkdir -p 'packages/self-use-neither/target'
cat <<END > packages/self-use-neither/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    self-use-neither/dep
    "
END

mkdir -p 'packages/self-use-neither/dep'
cat <<END > packages/self-use-neither/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
MYOPTIONS="disabled disabled2"
DEPENDENCIES="run: || ( self-use-neither/dep[disabled] self-use-neither/dep[disabled2] )"
END

# self-or-other
echo 'self-or-other' >> metadata/categories.conf

mkdir -p 'packages/self-or-other/target'
cat <<END > packages/self-or-other/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( self-or-other/target self-or-other/other )
    "
END

mkdir -p 'packages/self-or-other/other'
cat <<END > packages/self-or-other/other/other-1.exheres-0
SUMMARY="other"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# upgrade-over-any
echo 'upgrade-over-any' >> metadata/categories.conf

mkdir -p 'packages/upgrade-over-any/target'
cat <<END > packages/upgrade-over-any/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    || ( upgrade-over-any/dep[>=2] upgrade-over-any/dep[<2] )
    "
END

mkdir -p 'packages/upgrade-over-any/dep'
cat <<END > packages/upgrade-over-any/dep/dep-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# no-active-labels
echo 'no-active-labels' >> metadata/categories.conf

mkdir -p 'packages/no-active-labels/target'
cat <<END > packages/no-active-labels/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    test-expensive: || ( no-active-labels/dep no-active-labels/dep )
    "
END

mkdir -p 'packages/no-active-labels/dep'
cat <<END > packages/no-active-labels/dep/dep-1.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

cd ..

