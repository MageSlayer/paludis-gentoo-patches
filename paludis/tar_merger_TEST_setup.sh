#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir tar_merger_TEST_dir || exit 2
cd tar_merger_TEST_dir || exit 3

mkdir -p simple/subdir/subsubdir simple_extract
cat <<END > simple/file
This is the file.
END

cat <<END > simple/subdir/another
Another file
END

cat <<END > simple/subdir/subsubdir/script
Woohoo
END
chmod +x simple/subdir/subsubdir/script

