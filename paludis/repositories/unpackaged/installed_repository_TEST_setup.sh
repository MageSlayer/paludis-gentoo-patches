#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir installed_repository_TEST_dir || exit 1
cd installed_repository_TEST_dir || exit 1

mkdir -p root
mkdir -p repo1/indices/{categories/cat-one,packages/foo}
mkdir -p repo1/data/giant-space-weasel/{1:0:foo,2:1:bar}
ln -s ../../../data/giant-space-weasel repo1/indices/categories/cat-one/foo
ln -s ../../../data/giant-space-weasel repo1/indices/packages/foo/cat-one
cat <<"END" > repo1/data/giant-space-weasel/1:0:foo/contents
type=dir path=/fnord
END
cat <<"END" > repo1/data/giant-space-weasel/2:1:bar/contents
type=dir path=/stilton
type=file path=/stilton/cheese md5=1234567812345678 mtime=1234
type=file path=/stilton/is\ delicious md5=8765432187654321 mtime=2345
END
cat <<"END" > repo1/ndbam.conf
ndbam_format = 1
repository_format = installed_unpackaged-1
END

mkdir -p root2
cat <<"END" > root2/first
Eat me!
END
cat <<"END" > root2/second
I got changed.
END

mkdir -p repo2/indices/{categories/cat-one,packages/foo}
mkdir -p repo2/data/asdf/1.2.3:fred:ghjk
ln -s ../../../data/asdf repo2/indices/categories/cat-one/foo
ln -s ../../../data/asdf repo2/indices/packages/foo/cat-one
cat <<END > repo2/data/asdf/1.2.3:fred:ghjk/contents
type=file path=/first md5=c0ba8bfb6501abb1b7105ec79536b848 mtime=$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "root2/first")
type=file path=/first md5=0 mtime=$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "root2/second")
END
cat <<"END" > repo2/ndbam.conf
ndbam_format = 1
repository_format = installed_unpackaged-1
END

mkdir -p root3
mkdir -p repo3/indices/{categories/cat-one,packages/foo}
mkdir -p repo3/data/asdf/1.2.3:fred:ghjk
mkdir -p repo3/data/asdf/3.2.1:barney:qwerty
ln -s ../../../data/asdf repo3/indices/categories/cat-one/foo
ln -s ../../../data/asdf repo3/indices/packages/foo/cat-one
cat <<END > repo3/data/asdf/1.2.3:fred:ghjk/contents
END
cat <<"END" > repo3/ndbam.conf
ndbam_format = 1
repository_format = installed_unpackaged-1
END

mkdir -p root4
mkdir -p repo4

mkdir -p src4a/dir
echo "four a" > src4a/dir/4a

mkdir -p src4b1/dir
echo "four b 1" > src4b1/dir/4b
echo "four b 1" > src4b1/dir/4b1

mkdir -p src4b2/dir
echo "four b 2" > src4b2/dir/4b
echo "four b 2" > src4b2/dir/4b2

