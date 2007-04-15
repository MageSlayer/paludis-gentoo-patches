#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir set_file_TEST_dir || exit 2
cd set_file_TEST_dir

cat <<"END" > simple1
# this is a comment
  
foo/bar
  >=bar/baz-1.23

  # the end
END

cat <<"END" > paludisconf1
# this is a comment
  
? foo/bar
  * >=bar/baz-1.23

  # the end
END

