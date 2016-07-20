
set(CPACK_SET_DESTDIR TRUE)
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERISON_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERISON_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERISON_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
      "/build/"
      "/.git/"
      "\\\\.sw[opn]$"
      ".*~"
      "cscope.*"
      # TODO(compnerd) remove this set when the autotools build system is removed
      "/.gitignore"
      "/.gitreview"
      "/aclocal.m4"
      "/autogen.bash"
      "/autom4te.cache/"
      "/autotools_prepare.bash"
      "/config.h.in"
      "/configure.ac"
      "Makefile.am"
      "Makefile.am.m4"
      "Makefile.in"
      "/misc/common-makefile.am"
      "files.m4"
      "${CPACK_SOURCE_IGNORE_FILES}")

include(CPack)

add_custom_target(dist COMMAND "${CMAKE_MAKE_PROGRAM}" package_source)

