#!/bin/sh
# vim: set ft=sh sw=4 sts=4 et :

mkdir cran_repository_TEST_dir || exit 1
cd cran_repository_TEST_dir || exit 1

# repo1
mkdir -p repo1
cat <<EOF >> repo1/PACKAGES
Package: testpackage1
Version: 2006.05.08
Title: Test on a well formed PACKAGES file
Depends: R (>= 2.0.0)


Package: testpackage2
Version: 2006.05.08
Title: Test on a well formed PACKAGES file
EOF
cat <<EOF >> repo1/testpackage1.DESCRIPTION
Package: testpackage1
Version: 2006.05.08
Title: Test on a well formed PACKAGES file
Depends: R (>= 2.0.0)
Description: This description
     spans
     multiple lines without
     trailing backslashes or leading tabs :-/
License: Some weirdo license string
Packaged: Mon May 08 12:00:00 2006; kugelfang
EOF
cat <<EOF >> repo1/testpackage2.DESCRIPTION
Package: testpackage2
Version: 1
Title: 2nd Test Packages
Depends: R, testpackage1
License: Another weirdo license string
Packaged: Mon May 08 22:00:00 2006; kugelfang
EOF

mkdir -p repo2
cat <<EOF >> repo2/PACKAGES
Package: testbundle
Version: 1
Title: Testbundle for bundlepkg1 and bundlepkg2
Bundle: testbundle
Contains: bundlepkg1 bundlepkg2
EOF
cat <<EOF >> repo2/testbundle.DESCRIPTION
Bundle: testbundle
Version: 1
Date: 21 May 2006
Title: Testbundle for bundlepkg1 and bundlepkg2
Contains: bundlepkg1 bundlepkg2
License: Weird obfuscation of GPL
Packaged: Sun May 21 12:34:56 2006; kugelfang
EOF
