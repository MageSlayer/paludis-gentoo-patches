dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4. You should use the provided
dnl autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`action_queue',                      `hh', `cc', `test')
add(`attributes',                        `hh')
add(`clone',                             `hh', `impl')
add(`condition_variable',                `hh', `cc', `test')
add(`destringify',                       `hh', `cc', `test')
add(`dir_iterator',                      `hh', `cc', `test', `testscript')
add(`exception',                         `hh', `cc')
add(`fast_unique_copy',                  `hh', `test')
add(`fd_output_stream',                  `hh')
add(`fs_entry',                          `hh', `cc', `fwd', `test', `testscript')
add(`fd_holder',                         `hh')
add(`graph',                             `hh', `cc', `impl', `test')
add(`idle_action_pool',                  `hh', `cc', `test')
add(`iterator',                          `hh', `test')
add(`instantiation_policy',              `hh', `impl', `test')
add(`is_file_with_extension',            `hh', `cc', `se', `test', `testscript')
add(`join',                              `hh', `test')
add(`log',                               `hh', `cc', `se', `test')
add(`make_shared_ptr',                   `hh', `fwd')
add(`map',                               `hh', `fwd', `impl', `cc')
add(`mutex',                             `hh', `cc', `test')
add(`operators',                         `hh')
add(`options',                           `hh', `fwd', `cc', `test')
add(`output_wrapper',                    `test', `testscript')
add(`pipe',                              `hh', `cc')
add(`private_implementation_pattern',    `hh', `impl')
add(`pstream',                           `hh', `cc', `test')
add(`random',                            `hh', `cc', `test')
add(`sequence',                          `hh', `fwd', `impl')
add(`set',                               `hh', `fwd', `impl')
add(`save',                              `hh', `test')
add(`sr',                                `hh', `cc')
add(`stringify',                         `hh', `test')
add(`strip',                             `hh', `cc', `test')
add(`system',                            `hh', `cc', `test', `testscript')
add(`thread',                            `hh', `cc', `test')
add(`thread_pool',                       `hh', `cc', `test')
add(`tokeniser',                         `hh', `cc', `test')
add(`tr1_memory',                        `hh')
add(`tr1_type_traits',                   `hh')
add(`tr1_functional',                    `hh')
add(`util',                              `hh')
add(`validated',                         `hh', `fwd', `test')
add(`virtual_constructor',               `hh', `impl', `test')
add(`visitor',                           `hh', `impl', `fwd', `test')

