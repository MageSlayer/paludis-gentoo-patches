#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir portage_environment_TEST_dir || exit 2
cd portage_environment_TEST_dir || exit 3

mkdir -p profile
cat <<"END" > profile/make.defaults
ARCH="arch"
ACCEPT_KEYWORDS="arch"
USE_EXPAND="FOO_CARDS"
FOO_CARDS="four"
USE="foo_c"
END

mkdir -p repo/profiles
cat <<"END" > repo/profiles/repo_name
repo
END

mkdir -p query_use/${SYSCONFDIR}/portage
ln -s $(pwd )/profile query_use/${SYSCONFDIR}/make.profile
cat <<END > query_use/${SYSCONFDIR}/make.conf
USE="one two -three"
PORTDIR="`pwd`/repo"
END
cat <<"END" > query_use/${SYSCONFDIR}/portage/package.use
app/one -one four
END

mkdir -p accept_keywords/${SYSCONFDIR}/portage
ln -s $(pwd )/profile accept_keywords/${SYSCONFDIR}/make.profile
cat <<END > accept_keywords/${SYSCONFDIR}/make.conf
USE="one two -three"
PORTDIR="`pwd`/repo"
ACCEPT_KEYWORDS="other_arch"
END
cat <<"END" > accept_keywords/${SYSCONFDIR}/portage/package.keywords
app/one ~arch
app/two
app/three -*
app/four **
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
app/one -foo_cards_two foo_cards_three
END

