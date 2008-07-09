#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir config_file_TEST_dir || exit 2
cd config_file_TEST_dir || exit 3
echo "I am a fish." > config_file || exit 4
echo "I am a fish too." > unreadable_file || exit 5
chmod a-r unreadable_file || exit 6

cat <<"END" > sourced_one
one="cat"
two="dog"
three="fish"
source config_file_TEST_dir/sourced_two
four="sheep"
file="config_file_TEST_dir/sourced_two"
source "${file}"
two="dog"
END

cat <<"END" > sourced_two
two="chimp"
three="koala"
five="rabbit"
END

