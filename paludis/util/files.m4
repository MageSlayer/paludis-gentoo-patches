dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4. You should use the provided
dnl autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`attributes',                        `hh', `cc')
add(`compare',                           `hh', `cc')
add(`comparison_policy',                 `hh', `cc', `test')
add(`composite_pattern',                 `hh', `cc')
add(`container_entry',                   `hh', `cc', `test')
add(`counted_ptr',                       `hh', `cc', `test')
add(`create_insert_iterator',            `hh', `cc', `test')
add(`deleter',                           `hh', `cc', `test')
add(`destringify',                       `hh', `cc', `test')
add(`dir_iterator',                      `hh', `cc', `test', `testscript')
add(`exception',                         `hh', `cc')
add(`filter_insert_iterator',            `hh', `cc', `test')
add(`fs_entry',                          `hh', `cc', `test', `testscript')
add(`getenv',                            `hh', `cc', `test')
add(`indirect_iterator',                 `hh', `cc', `test')
add(`instantiation_policy',              `hh', `cc', `test')
add(`is_const',                          `hh', `cc', `test')
add(`is_file_with_extension',            `hh', `cc', `test', `testscript')
add(`iterator_utilities',                `hh', `cc', `test')
add(`join',                              `hh', `cc', `test')
add(`log',                               `hh', `cc', `test')
add(`private_implementation_pattern',    `hh', `cc')
add(`pstream',                           `hh', `cc', `test')
add(`save',                              `hh', `cc', `test')
add(`sequential_collection',             `hh', `cc')
add(`smart_record',                      `hh', `cc', `test')
add(`sorted_collection',                 `hh', `cc')
add(`stringify',                         `hh', `cc', `test')
add(`strip',                             `hh', `cc', `test')
add(`tokeniser',                         `hh', `cc', `test')
add(`transform_insert_iterator',         `hh', `cc', `test')
add(`util',                              `hh')
add(`validated',                         `hh', `cc', `test')
add(`virtual_constructor',               `hh', `cc', `test')
add(`visitor',                           `hh', `cc', `test')

