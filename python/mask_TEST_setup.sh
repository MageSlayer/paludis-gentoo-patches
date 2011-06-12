#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir mask_TEST_dir || exit 1
cd mask_TEST_dir || exit 1

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile,masked/{user,repo,unaccepted,unsupported}} || exit 1

cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
masked
END
cat <<END > profiles/profiles.desc
test testprofile stable
END
cat <<END > profiles/package.mask
masked/repo
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END

cat <<"END" > masked/repo/repo-1.0.ebuild || exit 1
DESCRIPTION="RepositoryMask"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE=""
LICENSE=""
KEYWORDS="test"
END

cat <<"END" > masked/user/user-1.0.ebuild || exit 1
DESCRIPTION="UserMask"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE=""
LICENSE=""
KEYWORDS="test"
END


cat <<"END" > masked/unaccepted/unaccepted-1.0.ebuild || exit 1
DESCRIPTION="UnacceptedMask"
HOMEPAGE="http://paludis.pioto.org/"
SRC_URI="http://example.com/${P}.tar.bz2"
SLOT="0"
IUSE=""
LICENSE=""
KEYWORDS="unaccepted"
END

cat <<"END" > masked/unsupported/unsupported-1.0.ebuild || exit 1
EAPI="unsupported"
END

cd ..

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = e
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/package_mask.conf
masked/user
END

cat <<END > home/.paludis/keywords.conf
*/* test
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

