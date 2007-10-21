#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir vdb_unmerger_TEST_dir || exit 2
cd vdb_unmerger_TEST_dir || exit 3

mkdir CONTENTS

mkdir root
cd root

make_file() {
    echo "foo" > "file_$1"

    md5=${2:-$(md5sum "file_$1" | cut -f1 -d' ')}
    mtime=${3:-$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "file_$1")}
    echo "obj /file_$1 ${md5} ${mtime}" > "../CONTENTS/file_$1"
}

make_sym() {
    src="sym_$1"
    dst=${2:-"sym_$1_dst"}

    > "sym_$1_dst"
    ln -s "${dst}" "${src}"

    mtime=${3:-$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "sym_$1")}
    echo "sym /${src} -> sym_$1_dst  ${mtime}" > "../CONTENTS/sym_$1"
}

make_file "ok"

make_file " with spaces"

make_file "bad_type"
rm file_bad_type
mkdir file_bad_type

make_file "bad_md5sum" "bad_md5sum"
make_file "bad_mtime" "" "123"
echo "obj /file_bad_entry foo" > ../CONTENTS/file_bad_entry

mkdir dir_ok
echo "dir /dir_ok" > ../CONTENTS/dir_ok

mkdir "dir_ with spaces"
echo "dir /dir_ with spaces" > "../CONTENTS/dir_ with spaces"

> dir_bad_type
echo "dir /dir_bad_type" > ../CONTENTS/dir_bad_type

mkdir -p dir_not_empty/foo
echo "dir /dir_not_empty" > ../CONTENTS/dir_not_empty

make_sym "ok"
make_sym " with spaces"

make_sym "bad_type"
rm sym_bad_type
> sym_bad_type

make_sym "bad_dst" "sym_dst_bad"
make_sym "bad_mtime" "" "123"

ln -s foo sym_bad_entry_1
echo "sym /sym_bad_entry_1 -> foo" > "../CONTENTS/sym_bad_entry_1"
ln -s foo sym_bad_entry_2
echo "sym /sym_bad_entry_2 >> foo 123" > "../CONTENTS/sym_bad_entry_2"

mkfifo fifo_ok
echo "fif /fifo_ok" > "../CONTENTS/fifo_ok"

mkfifo "fifo_ with spaces"
echo "fif /fifo_ with spaces" > "../CONTENTS/fifo_ with spaces"

> fifo_bad_type
echo "fif /fifo_bad_type" > "../CONTENTS/fifo_bad_type"

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

find . -name '*protected*' -type f -print | while read file; do
    echo obj "${file#.}" "$(md5sum "${file}" | cut -f1 -d' ')" "$(${PALUDIS_EBUILD_DIR}/utils/wrapped_getmtime "${file}")"
done >../CONTENTS/config_protect

