#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir find_stable_candidates_TEST_dir || exit 1
cd find_stable_candidates_TEST_dir || exit 1

mkdir -p {eclass,distfiles,profiles/profile} || exit 1
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
test profile stable
END

mkdir cat-one
mkdir cat-one/pkg-one
cat <<"END" > cat-one/pkg-one/pkg-one-1.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="best one"
END

cat <<"END" > cat-one/pkg-one/pkg-one-2.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="best"
END

mkdir cat-one/pkg-two
cat <<"END" > cat-one/pkg-two/pkg-two-1.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="foo"
END

cat <<"END" > cat-one/pkg-one/pkg-two-2.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="bar"
END

mkdir cat-one/pkg-three
cat <<"END" > cat-one/pkg-two/pkg-three-1.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="best one"
END

cat <<"END" > cat-one/pkg-one/pkg-three-2.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="best one"
END

mkdir cat-one/pkg-four
cat <<"END" > cat-one/pkg-two/pkg-four-1.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="best one"
END

cat <<"END" > cat-one/pkg-one/pkg-four-2.ebuild
DESCRIPTION="one"
SLOT="0"
KEYWORDS="~best ~one"
END


