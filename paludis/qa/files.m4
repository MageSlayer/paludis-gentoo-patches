dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4 and qa.hh.m4. You should
dnl use the provided autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`check',                          `hh', `cc')
add(`check_result',                   `hh', `cc', `test')
add(`environment',                    `hh', `cc')
add(`file_check',                     `hh', `cc')
add(`file_permissions_check',         `hh', `cc', `test', `testscript')
add(`message',                        `hh', `cc', `test')
add(`package_dir_check',              `hh', `cc')
add(`package_name_check',             `hh', `cc', `test', `testscript')
