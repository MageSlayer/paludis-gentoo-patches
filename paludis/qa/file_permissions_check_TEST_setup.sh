#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir file_permissions_check_TEST_dir || exit 2
cd file_permissions_check_TEST_dir || exit 3

touch ok_file no_read_file exec_file || exit 4
chmod -r no_read_file || exit 5
chmod +x exec_file || exit 6

mkdir ok_dir no_read_dir no_exec_dir || exit 7
chmod -r no_read_dir || exit 8
chmod -x no_exec_dir || exit 9

