dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4. You should use the provided
dnl autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`attributes',                        `hh')
add(`collection',                        `hh')
add(`compare',                           `hh')
add(`comparison_policy',                 `hh', `test')
add(`composite_pattern',                 `hh')
add(`container_entry',                   `hh', `test')
add(`counted_ptr',                       `hh', `cc', `test')
add(`deleter',                           `hh', `cc', `test')
add(`destringify',                       `hh', `cc', `test')
add(`dir_iterator',                      `hh', `cc', `test', `testscript')
add(`exception',                         `hh', `cc')
add(`fs_entry',                          `hh', `cc', `test', `testscript')
add(`iterator',                          `hh', `test')
add(`instantiation_policy',              `hh', `test')
add(`is_file_with_extension',            `hh', `cc', `test', `testscript')
add(`join',                              `hh', `test')
add(`log',                               `hh', `cc', `test')
add(`match_sequence',                    `hh', `cc', `test')
add(`private_implementation_pattern',    `hh')
add(`pstream',                           `hh', `cc', `test')
add(`random',                            `hh', `cc', `test')
add(`save',                              `hh', `test')
add(`smart_record',                      `hh', `test')
add(`stringify',                         `hh', `test')
add(`strip',                             `hh', `cc', `test')
add(`system',                            `hh', `cc', `test')
add(`tokeniser',                         `hh', `test')
add(`util',                              `hh')
add(`validated',                         `hh', `test')
add(`virtual_constructor',               `hh', `test')
add(`visitor',                           `hh', `test')

