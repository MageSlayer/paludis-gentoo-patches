dnl vim: set ft=m4 et :
dnl This file is used by Makefile.am.m4 and qa.hh.m4. You should
dnl use the provided autogen.bash script to do all the hard work.
dnl
dnl This file is used to avoid having to make lots of repetitive changes in
dnl Makefile.am every time we add a source or test file. The first parameter is
dnl the base filename with no extension; later parameters can be `hh', `cc',
dnl `test', `impl', `testscript'. Note that there isn't much error checking done
dnl on this file at present...

add(`changelog_check',                `hh', `cc')
add(`check',                          `hh', `cc')
add(`check_result',                   `hh', `cc', `test')
dnl add(`create_metadata_check',          `hh', `cc')
add(`defaults_check',                 `hh', `cc')
dnl add(`dep_any_check',                  `hh', `cc')
dnl add(`dep_flags_check',                `hh', `cc')
dnl add(`dep_packages_check',             `hh', `cc')
dnl add(`deps_exist_check',               `hh', `cc')
dnl add(`deps_visible_check',             `hh', `cc')
dnl add(`description_check',              `hh', `cc')
add(`digest_collisions_check',        `hh', `cc')
add(`ebuild_check',                   `hh', `cc')
add(`ebuild_count',                   `hh', `cc')
add(`environment',                    `hh', `cc')
dnl add(`extract_check',                  `hh', `cc')
add(`file_check',                     `hh', `cc')
add(`file_name_check',                `hh', `cc')
add(`file_permissions_check',         `hh', `cc', `test', `testscript')
add(`files_dir_size_check',           `hh', `cc')
add(`glep_31_check',                  `hh', `cc', `test')
add(`has_ebuilds_check',              `hh', `cc', `test', `testscript')
add(`has_misc_files_check',           `hh', `cc', `test', `testscript')
dnl add(`homepage_check',                 `hh', `cc')
dnl add(`inherits_check',                 `hh', `cc')
dnl add(`iuse_check',                     `hh', `cc')
dnl add(`keywords_check',                 `hh', `cc')
dnl add(`license_check',                  `hh', `cc')
add(`message',                        `hh', `cc', `test')
dnl add(`metadata_check',                 `hh', `cc')
add(`package_dir_check',              `hh', `cc')
add(`package_name_check',             `hh', `cc', `test', `testscript')
dnl add(`parse_deps_check',               `hh', `cc')
dnl add(`restrict_check',                 `hh', `cc')
dnl add(`src_uri_check',                  `hh', `cc')
dnl add(`pdepend_overlap_check',          `hh', `cc')
add(`qa',                             `hh', `cc')
dnl add(`slot_check',                     `hh', `cc')
add(`whitespace_check',               `hh', `cc')

