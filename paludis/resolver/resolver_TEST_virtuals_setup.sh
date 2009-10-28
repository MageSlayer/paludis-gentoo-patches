#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_virtuals_dir || exit 1
cd resolver_TEST_virtuals_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > profiles/categories
cat <<'END' > profiles/profile/make.defaults
ARCH=test
END
cat <<'END' > profiles/profile/virtuals
virtual/foo cat/foo-a
virtual/virtual-target cat/real-target
END

# common providers

echo 'cat' >> profiles/categories

mkdir -p 'cat/real-target'
cat <<END > cat/real-target/real-target-1.ebuild
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/virtual-target"
END

mkdir -p 'cat/foo-a'
cat <<END > cat/foo-a/foo-a-1.ebuild
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/foo"
END

mkdir -p 'cat/foo-b'
cat <<END > cat/foo-b/foo-b-1.ebuild
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/foo"
END

mkdir -p 'cat/foo-c'
cat <<END > cat/foo-c/foo-c-1.ebuild
DESCRIPTION="dep"
KEYWORDS="test"
SLOT="0"
PROVIDE="virtual/foo"
END

# virtuals
echo 'virtuals' >> profiles/categories

mkdir -p 'virtuals/target'
cat <<END > virtuals/target/target-1.ebuild
DESCRIPTION="target"
KEYWORDS="test"
SLOT="0"
DEPEND="
    ( virtual/foo )
    "
END

cd ..

