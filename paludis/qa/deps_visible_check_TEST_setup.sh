#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir deps_visible_check_TEST_dir || exit 1
cd deps_visible_check_TEST_dir || exit 1

mkdir -p repo1/{eclass,distfiles,profiles/profile} || exit 1
cd repo1 || exit 1
echo "test-repo-1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat-one
cat-two
cat-three
END
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > profiles/profiles.desc
test profile/ stable
END

mkdir -p cat-one/pkg-one
cat <<"END" > cat-one/pkg-one/pkg-one-1.ebuild
DESCRIPTION="foo"
SLOT="foo"
KEYWORDS="test"
END

cat <<"END" > cat-one/pkg-one/pkg-one-2.ebuild
DESCRIPTION="foo"
SLOT="foo"
KEYWORDS="test"
DEPEND="cat-two/fnord"
END

cat <<"END" > cat-one/pkg-one/pkg-one-3.ebuild
DESCRIPTION="foo"
SLOT="foo"
KEYWORDS="test"
DEPEND=">=cat-two/fnord-2"
END

mkdir -p cat-two/fnord
cat <<"END" > cat-two/fnord/fnord-1.ebuild
DESCRIPTION="foo"
SLOT="foo"
KEYWORDS="test"
END

cat <<"END" > cat-two/fnord/fnord-2.ebuild
DESCRIPTION="foo"
SLOT="foo"
KEYWORDS="~test"
END

cd ..

