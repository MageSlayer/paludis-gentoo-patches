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

: > profiles/categories
cat <<'END' > profiles/profile/make.defaults
ARCH=test
END

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
    ( !remove/a-pkg !remove/z-pkg )
    "
END

mkdir -p 'packages/remove/a-pkg'
cat <<END > packages/remove/a-pkg/a-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/remove/z-pkg'
cat <<END > packages/remove/z-pkg/z-pkg-2.exheres-0
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

# block-and-dep-cycle
echo 'block-and-dep-cycle' >> metadata/categories.conf

mkdir -p 'packages/block-and-dep-cycle/target'
cat <<END > packages/block-and-dep-cycle/target/target-1.ebuild
EAPI="2"
DESCRIPTION="target"
KEYWORDS="test"
SLOT="0"
DEPEND="
    block-and-dep-cycle/dep
    "
END

mkdir -p 'packages/block-and-dep-cycle/dep'
cat <<END > packages/block-and-dep-cycle/dep/dep-1.ebuild
EAPI="2"
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
DEPEND="
    !=block-and-dep-cycle/target-0
    "
END

# hard-block-and-dep-cycle
echo 'hard-block-and-dep-cycle' >> metadata/categories.conf

mkdir -p 'packages/hard-block-and-dep-cycle/target'
cat <<END > packages/hard-block-and-dep-cycle/target/target-1.ebuild
EAPI="2"
DESCRIPTION="target"
KEYWORDS="test"
SLOT="0"
DEPEND="
    hard-block-and-dep-cycle/dep
    "
END

mkdir -p 'packages/hard-block-and-dep-cycle/dep'
cat <<END > packages/hard-block-and-dep-cycle/dep/dep-1.ebuild
EAPI="2"
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
DEPEND="
    !!=hard-block-and-dep-cycle/target-0
    "
END

# self-block
for i in "x" "0" "1" ; do
    for d in "x" "0" "1" ; do
        for b in "w" "s" ; do
            cat=self-block-${i}-${d}-${b}

            dep=${cat}/dep

            if [[ "${d}" != "x" ]] ; then
                dep="=${dep}-${d}"
            fi

            if [[ "${b}" == "w" ]] ; then
                dep="!${dep}"
            else
                dep="!!${dep}"
            fi

            echo $cat >> metadata/categories.conf

            mkdir -p 'packages/'$cat'/target'
            cat <<END > packages/$cat/target/target-1.ebuild
EAPI="2"
DESCRIPTION="target"
KEYWORDS="test"
SLOT="0"
DEPEND="
    ${cat}/dep
    "
END

            mkdir -p 'packages/'$cat'/dep'
            cat <<END > packages/$cat/dep/dep-1.ebuild
EAPI="2"
DESCRIPTION="target"
KEYWORDS="test"
SLOT="0"
DEPEND="
    ${dep}
    "
END

        done
    done
done

# uninstall-blocked-after
echo 'uninstall-blocked-after' >> metadata/categories.conf

mkdir -p 'packages/uninstall-blocked-after/target'
cat <<END > packages/uninstall-blocked-after/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !uninstall-blocked-after/dep[=1] [[ resolution = uninstall-blocked-after ]] )
    "
END

mkdir -p 'packages/upgrade-blocked-after/dep'
cat <<END > packages/uninstall-blocked-after/dep/dep-2.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# uninstall-blocked-before
echo 'uninstall-blocked-before' >> metadata/categories.conf

mkdir -p 'packages/uninstall-blocked-before/target'
cat <<END > packages/uninstall-blocked-before/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !uninstall-blocked-before/dep[=1] [[ resolution = uninstall-blocked-before ]] )
    "
END

mkdir -p 'packages/upgrade-blocked-before/dep'
cat <<END > packages/uninstall-blocked-before/dep/dep-2.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# upgrade-blocked-before
echo 'upgrade-blocked-before' >> metadata/categories.conf

mkdir -p 'packages/upgrade-blocked-before/target'
cat <<END > packages/upgrade-blocked-before/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !upgrade-blocked-before/dep[=1] [[ resolution = upgrade-blocked-before ]] )
    "
END

mkdir -p 'packages/upgrade-blocked-before/dep'
cat <<END > packages/upgrade-blocked-before/dep/dep-2.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

# manual
echo 'manual' >> metadata/categories.conf

mkdir -p 'packages/manual/target'
cat <<END > packages/manual/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !manual/dep[=1] [[ resolution = manual ]] )
    "
END

mkdir -p 'packages/manual/dep'
cat <<END > packages/manual/dep/dep-2.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES=""
END

cd ..

