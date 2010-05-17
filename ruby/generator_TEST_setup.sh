#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir generator_TEST_dir || exit 1
cd generator_TEST_dir || exit 1

mkdir -p home/.paludis/repositories

cat <<END > home/.paludis/repositories/testrepo.conf
location = `pwd`/testrepo
format = ebuild
names_cache = /var/empty
cache = /var/empty
profiles = \${location}/profiles/testprofile
builddir = `pwd`
END

cat <<END > home/.paludis/general.conf
world = /dev/null
END

mkdir -p testrepo/{eclass,distfiles,profiles/testprofile} || exit 1
cd testrepo || exit 1
echo "testrepo" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
END
cat <<END > profiles/testprofile/make.defaults
ARCH=test
USERLAND=test
KERNEL=test
END
cat <<END > profiles/profiles.desc
test testprofile stable
END

