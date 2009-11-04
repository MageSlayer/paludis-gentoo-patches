#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_dependencies_rewriter_dir || exit 1
cd e_repository_TEST_dependencies_rewriter_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir
echo "already fetched" > distdir/already-fetched.txt || exit 1
cat <<END > distdir/expatch-success-1.patch || exit 1
--- a/bar
+++ b/bar
@@ -1 +1,3 @@
 foo
+bar
+baz
END

mkdir -p repo/{eclass,distfiles,profiles/profile} || exit 1
mkdir -p repo/category/package || exit 1
cd repo || exit 1
echo "test-repo" >> profiles/repo_name || exit 1
echo "category" >> profiles/categories || exit 1
cat <<END > profiles/profile/make.defaults
ARCH=test
END
cat <<END > category/package/package-1.ebuild || exit 1
EAPI="exheres-0"
SLOT="0"
PLATFORMS="test"
DEPENDENCIES="cat/pkg1 build: cat/pkg2 build+run: cat/pkg3 suggestion: cat/pkg4 post: cat/pkg5"
END
cd ..

cd ..

