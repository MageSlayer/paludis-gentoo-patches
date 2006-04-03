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
add(`config_file',                       `hh', `cc', `test', `testscript')
add(`default_config',                    `hh', `cc')
add(`default_environment',               `hh', `cc')
add(`dep_atom',                          `hh', `cc', `test')
add(`dep_atom_dumper',                   `hh', `cc', `test')
add(`dep_atom_flattener',                `hh', `cc')
add(`dep_atom_pretty_printer',           `hh', `cc')
add(`dep_lexer',                         `hh', `cc', `test')
add(`dep_list',                          `hh', `cc', `test')
add(`dep_parser',                        `hh', `cc', `test')
add(`environment',                       `hh', `cc')
add(`fake_repository',                   `hh', `cc')
add(`hashed_containers',                 `hh', `cc', `test')
add(`mask_reasons',                      `hh', `cc')
add(`match_package',                     `hh', `cc')
add(`name',                              `hh', `cc', `test')
add(`package_database',                  `hh', `cc', `test')
add(`paludis',                           `hh', `cc')
add(`portage_repository',                `hh', `cc')
add(`repository',                        `hh', `cc')
add(`test_environment',                  `hh', `cc')
add(`vdb_repository',                    `hh', `cc')
add(`version_metadata',                  `hh', `cc')
add(`version_operator',                  `hh', `cc', `test')
add(`version_spec',                      `hh', `cc', `test')

