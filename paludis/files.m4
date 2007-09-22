dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4 and paludis.hh.m4. You should
dnl use the provided autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`about',                             `hh', `test')
add(`action',                            `hh', `cc', `fwd', `se', `sr')
add(`config_file',                       `hh', `cc', `se', `test', `testscript')
add(`contents',                          `hh', `cc', `fwd')
add(`dep_label',                         `hh', `cc', `fwd')
add(`dep_spec',                          `hh', `cc', `se', `test', `fwd')
add(`dep_spec_flattener',                `hh', `cc')
add(`dep_tag',                           `hh', `cc', `fwd', `sr')
add(`distribution',                      `hh', `cc', `fwd', `sr')
add(`environment',                       `hh', `fwd', `cc', `se')
add(`environment_implementation',        `hh', `cc')
add(`formatter',                         `hh', `fwd', `cc')
add(`hashed_containers',                 `hh', `cc', `test')
add(`hook',                              `hh', `cc', `fwd',`se', `sr')
add(`hooker',                            `hh', `cc', `test', `testscript')
add(`host_tuple_name',                   `hh', `cc', `sr', `test')
add(`mask',                              `hh', `cc', `fwd', `sr')
add(`match_package',                     `hh', `cc')
add(`metadata_key',                      `hh', `cc', `se', `fwd')
add(`name',                              `hh', `cc', `fwd', `test', `sr', `se')
add(`package_database',                  `hh', `cc', `fwd', `test', `se')
add(`package_id',                        `hh', `cc', `fwd', `se')
add(`paludis',                           `hh')
add(`qa',                                `hh', `cc', `fwd', `se', `sr')
add(`query',                             `hh', `cc', `fwd')
add(`repository',                        `hh', `fwd', `cc', `sr')
add(`repository_info',                   `hh', `fwd', `cc')
add(`repository_name_cache',             `hh', `cc', `test', `testscript')
add(`set_file',                          `hh', `cc', `se', `sr', `test', `testscript')
add(`stringify_formatter',               `hh', `cc', `fwd', `impl', `test')
add(`syncer',                            `hh', `cc', `sr')
add(`version_operator',                  `hh', `cc', `fwd', `se', `test')
add(`version_requirements',              `hh', `cc', `fwd', `sr')
add(`version_spec',                      `hh', `cc', `sr', `fwd', `test')

