#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_subslots_dir || exit 1
cd resolver_TEST_subslots_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
:> metadata/categories.conf

# subslot
echo 'subslot' >> metadata/categories.conf

mkdir -p 'packages/subslot/dependency'
cat <<END > packages/subslot/dependency/dependency-1.1.ebuild
EAPI="5"
SLOT="0/1"
KEYWORDS="test"
DEPEND=""
END
cat <<END > packages/subslot/dependency/dependency-2.ebuild
EAPI="5"
SLOT="0/2"
KEYWORDS="test"
DEPEND=""
END

mkdir -p 'packages/subslot/uses-library'
cat <<END > packages/subslot/uses-library/uses-library-1.ebuild
EAPI="5"
SLOT="0"
KEYWORDS="test"
DEPEND="subslot/dependency:="
END

mkdir -p 'packages/subslot/uses-tool'
cat <<END > packages/subslot/uses-tool/uses-tool-1.ebuild
EAPI="5"
SLOT="0"
KEYWORDS="test"
DEPEND="subslot/dependency:*"
END

cd ..

