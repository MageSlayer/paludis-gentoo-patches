#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir portage_environment_TEST_dir || exit 2
cd portage_environment_TEST_dir || exit 3

mkdir -p profile
cat <<END > profile/make.defaults
ROOT="`pwd`"
ARCH="arch"
ACCEPT_KEYWORDS="arch"
USE_EXPAND="FOO_CARDS BAR_CARDS"
FOO_CARDS="four"
BAR_CARDS="monkey"
USE="foo_c six"
END

mkdir -p var/db/pkg

mkdir -p repo/{profiles,cat-one/pkg-{one,two,three,four,five,x}}
cat <<"END" > repo/profiles/repo_name
repo
END
echo cat-one > repo/profiles/categories
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-one/pkg-one-1.ebuild || exit 4
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-two/pkg-two-1.ebuild || exit 4
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-three/pkg-three-1.ebuild || exit 4
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-four/pkg-four-1.ebuild || exit 4
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-five/pkg-five-1.ebuild || exit 4
echo -e "IUSE='one two three four five six'\nSLOT='0'\n" > repo/cat-one/pkg-x/pkg-x-1.ebuild || exit 4

mkdir -p query_use/${SYSCONFDIR}/portage
ln -s $(pwd )/profile query_use/${SYSCONFDIR}/make.profile
cat <<END > query_use/${SYSCONFDIR}/make.conf
USE="one two -three -six"
PORTDIR="`pwd`/repo"
BAR_CARDS="bar"
END
cat <<"END" > query_use/${SYSCONFDIR}/portage/package.use
cat-one/pkg-one -one four
END

mkdir -p accept_keywords/${SYSCONFDIR}/portage
ln -s $(pwd )/profile accept_keywords/${SYSCONFDIR}/make.profile
cat <<END > accept_keywords/${SYSCONFDIR}/make.conf
USE="one two -three"
PORTDIR="`pwd`/repo"
ACCEPT_KEYWORDS="other_arch"
END
cat <<"END" > accept_keywords/${SYSCONFDIR}/portage/package.keywords
cat-one/pkg-one ~arch
cat-one/pkg-two
cat-one/pkg-three -*
cat-one/pkg-four **
cat-one/pkg-five ~*
END

mkdir -p known_use_expand_names/${SYSCONFDIR}/portage
ln -s $(pwd )/profile known_use_expand_names/${SYSCONFDIR}/make.profile
cat <<END > known_use_expand_names/${SYSCONFDIR}/make.conf
USE="one two -three foo"
PORTDIR="`pwd`/repo"
ACCEPT_KEYWORDS="other_arch"
FOO_CARDS="one"
END
cat <<"END" > known_use_expand_names/${SYSCONFDIR}/portage/package.use
cat-one/pkg-one -foo_cards_two foo_cards_three
END

mkdir -p world/{${SYSCONFDIR},var/lib/portage}
ln -s $(pwd )/profile world/${SYSCONFDIR}/make.profile
cat <<END > world/${SYSCONFDIR}/make.conf
PORTDIR="`pwd`/repo"
END
cat <<END > world/var/lib/portage/world
cat/unchanged
cat/before
cat/alsounchanged
END


