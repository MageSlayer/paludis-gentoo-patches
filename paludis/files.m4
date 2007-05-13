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
add(`config_file',                       `hh', `cc', `se', `test', `testscript')
add(`contents',                          `hh', `cc')
add(`dep_spec',                          `hh', `cc', `se', `test')
add(`dep_spec_flattener',                `hh', `cc')
add(`dep_spec_pretty_printer',           `hh', `cc', `test')
add(`dep_tag',                           `hh', `cc', `sr')
add(`environment',                       `hh', `cc', `se')
add(`environment_implementation',        `hh', `cc', `test')
add(`hashed_containers',                 `hhx', `cc', `test')
add(`hook',                              `hh', `cc')
add(`hooker',                            `hh', `cc', `test', `testscript')
add(`host_tuple_name',                   `hh', `cc', `sr', `test')
add(`mask_reasons',                      `hh', `cc', `se')
add(`match_package',                     `hh', `cc')
add(`name',                              `hh', `cc', `test', `sr', `se')
add(`package_database',                  `hh', `cc', `test', `se')
add(`package_database_entry',            `hh', `cc', `sr')
add(`paludis',                           `hh', `cc')
add(`portage_dep_lexer',                 `hh', `cc', `test')
add(`portage_dep_parser',                `hh', `cc', `test')
add(`query',                             `hh', `cc')
add(`repository',                        `hh', `fwd', `cc', `sr')
add(`repository_name_cache',             `hh', `cc', `test', `testscript')
add(`set_file',                          `hh', `cc', `se', `sr', `test', `testscript')
add(`syncer',                            `hh', `cc', `sr')
add(`version_metadata',                  `hh', `cc', `sr')
add(`version_operator',                  `hh', `cc', `se', `test')
add(`version_requirements',              `hh', `cc', `sr')
add(`version_spec',                      `hh', `cc', `sr', `test')

