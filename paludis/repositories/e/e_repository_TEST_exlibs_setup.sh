#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir e_repository_TEST_exlibs_dir || exit 1
cd e_repository_TEST_exlibs_dir || exit 1

mkdir -p root/etc

mkdir -p vdb
touch vdb/THISISTHEVDB

mkdir -p build
ln -s build symlinked_build

mkdir -p distdir

mkdir -p repo1/{profiles/profile,metadata,eclass} || exit 1
cd repo1 || exit 1
echo "test-repo-1" >> profiles/repo_name || exit 1
echo "cat" >> metadata/categories.conf || exit 1
cat <<END > profiles/profile/make.defaults
CHOST="i286-badger-linux-gnu"
SUBOPTIONS="LINGUAS"
LINGUAS="en en_GB en_GB@UTF-8"
USERLAND="GNU"
OPTIONS="weasel spinach"
END
mkdir -p "packages/cat/require-success"
cat <<'END' > packages/cat/require-success/foo.exlib || exit 1
DESCRIPTION="The Long Description"
SUMMARY="The Short Description"
HOMEPAGE="http://example.com/"
SLOT="0"
LICENCES="GPL-2"
export_exlib_phases src_unpack

foo_src_unpack() {
    [[ ${FOO} == true ]] || die "FOO unset"
    touch ${PN}.foo
}
END
cat <<'END' > packages/cat/require-success/require-success-1.ebuild || exit 1
require foo
PLATFORMS="test"
FOO=true

src_install() {
    [[ ${SUMMARY} == "The Short Description" ]] || die "Bad SUMMARY"
    [[ -e ${PN}.foo ]] || die "foo_src_unpack did not run"
}
END
mkdir -p "packages/cat/require-fail"
cat <<'END' > packages/cat/require-fail/require-fail-1.ebuild || exit 1
require non-existant
PLATFORMS="test"
END
mkdir -p "exlibs"
cat <<'END' > exlibs/foo.exlib || exit 1
SUMMARY="Short description"
SLOT="0"
END
cat <<'END' > exlibs/foo-bar.exlib || exit 1
SLOT="0"
myexparam bar
SUMMARY=$(exparam bar)
END
cat <<'END' > exlibs/foo-bar-baz.exlib || exit 1
SLOT="0"
myexparam bar=baz
SUMMARY=$(exparam bar)
END
cat <<'END' > exlibs/monkey.exlib
myexparam blah
DESCRIPTION=$(exparam blah)
END
mkdir -p "packages/cat/require-param"
cat <<'END' > packages/cat/require-param/require-param-1.ebuild || exit 1
require foo-bar [ bar=baz ]
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == baz ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/require-param-empty"
cat <<'END' > packages/cat/require-param-empty/require-param-empty-1.ebuild || exit 1
require foo-bar [ bar= ]
PLATFORMS="test"

pkg_setup() {
    [[ -z ${SUMMARY} && ${SUMMARY+set} == set ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/require-param-missing"
cat <<'END' > packages/cat/require-param-missing/require-param-missing-1.ebuild || exit 1
require foo-bar
PLATFORMS="test"
END
mkdir -p "packages/cat/require-param-undeclared"
cat <<'END' > packages/cat/require-param-undeclared/require-param-undeclared-1.ebuild || exit 1
require foo [ bar=baz ]
PLATFORMS="test"
END
mkdir -p "packages/cat/require-params"
cat <<'END' > packages/cat/require-params/require-params-1.ebuild || exit 1
require foo-bar [[[ bar=baz ]]] monkey [ blah=bleh ]
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == baz ]] || die "Bad SUMMARY"
    [[ ${DESCRIPTION} == bleh ]] || die "Bad DESCRIPTION"
}
END
mkdir -p "packages/cat/require-params-unaligned"
cat <<'END' > packages/cat/require-params-unaligned/require-params-unaligned-1.ebuild || exit 1
require foo-bar [[[ bar=baz ] monkey [ blah=bleh ]
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == baz ]] || die "Bad SUMMARY"
    [[ ${DESCRIPTION} == bleh ]] || die "Bad DESCRIPTION"
}
END
mkdir -p "packages/cat/require-multiple-params"
cat <<'END' > packages/cat/require-multiple-params/foo.exlib || exit 1
myexparam foo
myexparam bar
SUMMARY=$(exparam foo)$(exparam bar)
END
cat <<'END' > packages/cat/require-multiple-params/require-multiple-params-1.ebuild || exit 1
require foo [ foo=1 bar=2 ]
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == 12 ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/require-multiple-params-spaces"
cat <<'END' > packages/cat/require-multiple-params-spaces/foo.exlib || exit 1
myexparam foo
myexparam bar
SUMMARY=$(exparam foo)-$(exparam bar)
END
cat <<'END' > packages/cat/require-multiple-params-spaces/require-multiple-params-spaces-1.ebuild || exit 1
require foo [ foo="f o o " bar=" b a r" ]
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == "f o o - b a r" ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/require-param-default"
cat <<'END' > packages/cat/require-param-default/require-param-default-1.ebuild || exit 1
require foo-bar-baz
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == baz ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/require-multiple-params-default-spaces"
cat <<'END' > packages/cat/require-multiple-params-default-spaces/foo.exlib || exit 1
myexparam foo="f o o "
myexparam bar=" b a r"
SUMMARY=$(exparam foo)-$(exparam bar)
END
cat <<'END' > packages/cat/require-multiple-params-default-spaces/require-multiple-params-default-spaces-1.ebuild || exit 1
require foo
PLATFORMS="test"

pkg_setup() {
    [[ ${SUMMARY} == "f o o - b a r" ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/exparam-banned"
cat <<'END' > packages/cat/exparam-banned/exparam-banned-1.ebuild || exit 1
PLATFORMS="test"
exparam foo
END
mkdir -p "packages/cat/exparam-undeclared"
cat <<'END' > packages/cat/exparam-undeclared/foo.exlib || exit 1
get_bar() {
    exparam bar
}
END
cat <<'END' > packages/cat/exparam-undeclared/exparam-undeclared-1.ebuild || exit 1
require foo
PLATFORMS="test"

pkg_setup() {
    [[ $(get_bar) == baz ]]
}
END
mkdir -p "packages/cat/exparam-subshell"
cat <<'END' > packages/cat/exparam-subshell/foo.exlib || exit 1
myexparam bar
check_bar() {
    [[ $(exparam bar) == baz ]] || die "exparam in subshell failed"
}
END
cat <<'END' > packages/cat/exparam-subshell/exparam-subshell-1.ebuild || exit 1
require foo [ bar=baz ]

pkg_setup() {
    check_bar
}
END
mkdir -p "packages/cat/exarray"
cat <<'END' > packages/cat/exarray/foo.exlib || exit 1
myexparam bar[]

check_foo() {
    exparam bar[#] | grep -q ^3$ || die "Bad bar[#]"
    exparam bar[0] | grep -q ^1$ || die "Bad bar[0]"
    exparam bar[1] | grep -q ^2$ || die "Bad bar[1]"
    exparam bar[2] | grep -q ^3$ || die "Bad bar[2]"
    exparam bar[3] | grep -q ^$ || die "Bad bar[3]"
    exparam -v FOOA  bar[@]
    exparam -v FOO   bar
    exparam -v FOO0  bar[0]
    exparam -v FOO1  bar[1]
    exparam -v FOO2  bar[2]
    exparam -v FOO3  bar[3]
    exparam -v FOOC  bar[#]
}
END
cat <<'END' > packages/cat/exarray/exarray-1.ebuild || exit 1
require foo [ bar=[ 1 2 3 ] ]
PLATFORMS="test"

pkg_setup() {
    check_foo || die "check_foo returned errror"
    [[ ${#FOOA[@]} -eq 3 ]] || die "Wrong number of elements, ${#FOOA[@]} in FOOA[@]"
    [[ ${FOOA[0]} == 1 ]] || die "Bad FOOA[0]"
    [[ ${FOOA[1]} == 2 ]] || die "Bad FOOA[1]"
    [[ ${FOOA[2]} == 3 ]] || die "Bad FOOA[2]"
    [[ ${FOOA[@]} == "1 2 3" ]] || die "FOOA[@] != 1 2 3"
    [[ ${FOO} == 1 ]] || die "Bad FOO"
    [[ ${FOO0} == 1 ]] || die "Bad FOO0"
    [[ ${FOO1} == 2 ]] || die "Bad FOO1"
    [[ ${FOO2} == 3 ]] || die "Bad FOO2"
    [[ -z ${FOO3} ]] || die "Bad FOO3"
    [[ ${FOOC} -eq 3 ]] || die "Bad FOOC"
}
END
mkdir -p "packages/cat/exarray-spaces"
cat <<'END' > packages/cat/exarray-spaces/foo.exlib || exit 1
myexparam bar[]

check_foo() {
    exparam bar[#] | grep -q ^3$ || die "Bad bar[#]"
    exparam bar[0] | grep -q '^1 1$' || die "Bad bar[0]"
    exparam bar[1] | grep -q '^2 2$' || die "Bad bar[1]"
    exparam bar[2] | grep -q '^3 3$' || die "Bad bar[2]"
    exparam bar[3] | grep -q ^$ || die "Bad bar[3]"
    exparam -v FOO bar[@]
}
END
cat <<'END' > packages/cat/exarray-spaces/exarray-spaces-1.ebuild || exit 1
require foo [ bar=[ "1 1" "2 2" "3 3" ] ]
PLATFORMS="test"

pkg_setup() {
    check_foo || die "check_foo returned errror"
    [[ ${#FOO[@]} -eq 3 ]] || die "Wrong number of elements, ${#FOO[@]} in FOO[@]"
    [[ ${FOO[0]} == "1 1" ]] || die "Bad FOO[0]"
    [[ ${FOO[1]} == "2 2" ]] || die "Bad FOO[1]"
    [[ ${FOO[2]} == "3 3" ]] || die "Bad FOO[2]"
    [[ ${FOO[@]} == "1 1 2 2 3 3" ]] || die "FOO[@] != 1 1 2 2 3 3"
}
END
mkdir -p "packages/cat/exarray-default"
cat <<'END' > packages/cat/exarray-default/foo.exlib || exit 1
myexparam bar=[ 1 2 3 ]

check_foo() {
    exparam bar[#] | grep -q ^3$ || die "Bad bar[#]"
    exparam bar[0] | grep -q ^1$ || die "Bad bar[0]"
    exparam bar[1] | grep -q ^2$ || die "Bad bar[1]"
    exparam bar[2] | grep -q ^3$ || die "Bad bar[2]"
    exparam bar[3] | grep -q ^$ || die "Bad bar[3]"
    exparam -v FOO bar[@]
}
END
cat <<'END' > packages/cat/exarray-default/exarray-default-1.ebuild || exit 1
require foo
PLATFORMS="test"

pkg_setup() {
    check_foo || die "check_foo returned errror"
    [[ ${#FOO[@]} -eq 3 ]] || die "Wrong number of elements, ${#FOO[@]} in FOO[@]"
    [[ ${FOO[0]} == 1 ]] || die "Bad FOO[0]"
    [[ ${FOO[1]} == 2 ]] || die "Bad FOO[1]"
    [[ ${FOO[2]} == 3 ]] || die "Bad FOO[2]"
    [[ ${FOO[@]} == "1 2 3" ]] || die "FOO[@] != 1 2 3"
}
END
mkdir -p "packages/cat/exarray-default-spaces"
cat <<'END' > packages/cat/exarray-default-spaces/foo.exlib || exit 1
myexparam bar=[ "1 1" "2 2" "3 3" ]

check_foo() {
    exparam bar[#] | grep -q ^3$ || die "Bad bar[#]"
    exparam bar[0] | grep -q '^1 1$' || die "Bad bar[0]"
    exparam bar[1] | grep -q '^2 2$' || die "Bad bar[1]"
    exparam bar[2] | grep -q '^3 3$' || die "Bad bar[2]"
    exparam bar[3] | grep -q ^$ || die "Bad bar[3]"
    exparam -v FOO bar[@]
}
END
cat <<'END' > packages/cat/exarray-default-spaces/exarray-default-spaces-1.ebuild || exit 1
require foo
PLATFORMS="test"

pkg_setup() {
    check_foo || die "check_foo returned errror"
    [[ ${#FOO[@]} -eq 3 ]] || die "Wrong number of elements, ${#FOO[@]} in FOO[@]"
    [[ ${FOO[0]} == "1 1" ]] || die "Bad FOO[0]"
    [[ ${FOO[1]} == "2 2" ]] || die "Bad FOO[1]"
    [[ ${FOO[2]} == "3 3" ]] || die "Bad FOO[2]"
    [[ ${FOO[@]} == "1 1 2 2 3 3" ]] || die "FOO[@] != 1 1 2 2 3 3"
}
END
mkdir -p "packages/cat/noarray"
cat <<'END' > packages/cat/noarray/foo.exlib || exit 1
myexparam bar
SUMMARY=$(exparam bar)
END
cat <<'END' > packages/cat/noarray/noarray-1.ebuild || exit 1
require foo [[ bar=[ ]]

pkg_setup() {
    [[ ${SUMMARY} == "[" ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/noarray-bad"
cat <<'END' > packages/cat/noarray-bad/foo.exlib || exit 1
myexparam bar
SUMMARY=$(exparam bar)
END
cat <<'END' > packages/cat/noarray-bad/noarray-bad-1.ebuild || exit 1
require foo [ bar=[ ]

pkg_setup() {
    [[ ${SUMMARY} == "[" ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/scalar-required"
cat <<'END' > packages/cat/scalar-required/foo.exlib || exit 1
myexparam bar
SUMMARY=$(exparam bar)
END
cat <<'END' > packages/cat/scalar-required/scalar-required-1.ebuild || exit 1
require foo [ bar=[ baz ] ]

pkg_setup() {
    [[ ${SUMMARY} == "baz" ]] || die "Bad SUMMARY"
}
END
mkdir -p "packages/cat/array-required"
cat <<'END' > packages/cat/array-required/foo.exlib || exit 1
myexparam bar[]
SUMMARY=$(exparam bar)
END
cat <<'END' > packages/cat/array-required/array-required-1.ebuild || exit 1
require foo [ bar=baz ]

pkg_setup() {
    [[ ${SUMMARY} == "baz" ]] || die "Bad SUMMARY"
}
END
cd ..

cd ..

