#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir vdb_unmerger_TEST_dir || exit 2
cd vdb_unmerger_TEST_dir || exit 3

mkdir -p repo/cat
mkdir root
cd root

fix() {
    tr ' \t' '__' <<<"$1"
}

make_vdb() {
    mkdir -p "../repo/cat/$1-1234"
    for i in SLOT EAPI ; do
        echo "0" > "../repo/cat/$1-1234/${i}"
    done
    for i in DEPEND RDEPEND LICENSE INHERITED IUSE PDEPEND PROVIDE ; do
        touch "../repo/cat/$1-1234/${i}"
    done
}

make_file() {
    echo "foo" > "file_$1"

    md5=${2:-$(md5sum "file_$1" | cut -f1 -d' ')}
    mtime=${3:-$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "file_$1")}

    make_vdb "file_$(fix "$1" )"
    echo "obj /file_$1 ${md5} ${mtime}" > "../repo/cat/file_$(fix "$1" )-1234/CONTENTS"
}

make_sym() {
    src="sym_$1"
    dst=${2:-"sym_$1_dst"}

    > "sym_$1_dst"
    ln -s "${dst}" "${src}"

    mtime=${3:-$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "sym_$1")}

    make_vdb "sym_$(fix "$1" )"
    echo "sym /${src} -> sym_$1_dst ${mtime}" > "../repo/cat/sym_$(fix "$1" )-1234/CONTENTS"
}

make_file "ok"

make_file " with spaces"

make_file " with lots  of   spaces"

make_file " with trailing  space	 "

make_file "bad_type"
rm file_bad_type
mkdir file_bad_type

make_file "bad_md5sum" "bad_md5sum"
make_file "bad_mtime" "" "123"

make_vdb file_bad_entry
echo "obj /file_bad_entry foo" > ../repo/cat/file_bad_entry-1234/CONTENTS

make_vdb file_replaces_dir
echo "dir /file_replaces_dir" > ../repo/cat/file_replaces_dir-1234/CONTENTS
echo "obj /file_replaces_dir/foo 00000000000000000000000 123" >> ../repo/cat/file_replaces_dir-1234/CONTENTS
> file_replaces_dir

make_vdb dir_ok
mkdir dir_ok
echo "dir /dir_ok" > ../repo/cat/dir_ok-1234/CONTENTS

make_vdb "dir__with_spaces"
mkdir "dir_ with spaces"
echo "dir /dir_ with spaces" > "../repo/cat/dir__with_spaces-1234/CONTENTS"

make_vdb "dir__with_lots__of___spaces"
mkdir "dir_ with lots  of   spaces"
echo "dir /dir_ with lots  of   spaces" > "../repo/cat/dir__with_lots__of___spaces-1234/CONTENTS"

make_vdb "dir_bad_type"
> dir_bad_type
echo "dir /dir_bad_type" > ../repo/cat/dir_bad_type-1234/CONTENTS

make_vdb "dir_not_empty"
mkdir -p dir_not_empty/foo
echo "dir /dir_not_empty" > ../repo/cat/dir_not_empty-1234/CONTENTS

make_sym "ok"
make_sym " with spaces"
make_sym " with lots  of   spaces"

make_vdb sym_with_many_arrows
> "sym -> with -> many -> arrows -> dst"
ln -s "sym -> with -> many -> arrows -> dst" "sym with many arrows"
mtime=${3:-$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "sym with many arrows")}
echo "sym /sym with many arrows -> sym -> with -> many -> arrows -> dst ${mtime}" > "../repo/cat/sym_with_many_arrows-1234/CONTENTS"

make_sym "bad_type"
rm sym_bad_type
> sym_bad_type

make_sym "bad_dst" "sym_dst_bad"
make_sym "bad_mtime" "" "123"

make_vdb sym_bad_entry_1
ln -s foo sym_bad_entry_1
echo "sym /sym_bad_entry_1 -> foo" > "../repo/cat/sym_bad_entry_1-1234/CONTENTS"

make_vdb sym_bad_entry_2
ln -s foo sym_bad_entry_2
echo "sym /sym_bad_entry_2 >> foo 123" > "../repo/cat/sym_bad_entry_2-1234/CONTENTS"

touch protected_file
touch unprotected_file
touch protected_file_not_really

mkdir protected_dir
touch protected_dir/protected_file
touch protected_dir/unprotected_file
touch protected_dir/unprotected_file_not_really

mkdir protected_dir/unprotected_dir
touch protected_dir/unprotected_dir/unprotected_file

mkdir protected_dir/unprotected_dir_not_really
touch protected_dir/unprotected_dir_not_really/protected_file

mkdir protected_dir_not_really
touch protected_dir_not_really/unprotected_file

make_vdb config_protect
find . -name '*protected*' -type f -print | while read file; do
    echo obj "${file#.}" "$(md5sum "${file}" | cut -f1 -d' ')" "$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "${file}")"
done >../repo/cat/config_protect-1234/CONTENTS

