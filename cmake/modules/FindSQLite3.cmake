#.rst:
# FindSQLite3
# ------------
#
# Find SQLite3 library and headers
#
# The module defines the following variables:
#
# ::
#
#   SQLite3_FOUND       - true if SQLite3 was found
#   SQLite3_INCLUDE_DIR - include search path
#   SQLite3_LIBRARIES   - libraries to link
#   SQLite3_VERSION     - libmagic 3-component version number

if(SQLite3_INCLUDE_DIRS AND SQLite3_LIBRARIES)
  set(SQLite3_FOUND TRUE)
else()
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_SQLITE3 QUIET sqlite3)

  find_path(SQLite3_INCLUDE_DIR
            NAMES
              sqlite3.h
            HINTS
              ${PC_SQLITE3_INCLUDEDIR}
              ${PC_SQLITE3_INCLUDE_DIRS}
              ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  find_library(SQLite3_LIBRARIES
               NAMES
                 sqlite3 libsqlite3
               HINTS
                 ${PC_SQLITE3_LIBDIR}
                 ${PC_SQLITE3_LIBRARY_DIRS}
                 ${CMAKE_INSTALL_FULL_LIBDIR})

  if(SQLite3_INCLUDE_DIR AND EXISTS "${SQLite3_INCLUDE_DIR}/sqlite3.h")
    file(STRINGS "${SQLite3_INCLUDE_DIR}/sqlite3.h"
         sqlite3_version_str
         REGEX "^#define SQLITE_VERSION[ ]+\".*\"")
     string(REGEX REPLACE "^#define SQLITE_VERSION[ ]+\"([^\"]*)\".*" "\\1"
            SQLite3_VERSION_STRING "${sqlite3_version_str}")
     unset(sqlite3_version_str)
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SQLite3
                                    REQUIRED_VARS
                                      SQLite3_LIBRARIES
                                      SQLite3_INCLUDE_DIR
                                    VERSION_VAR
                                      SQLite3_VERSION_STRING)
  mark_as_advanced(SQLite3_INCLUDE_DIR SQLite3_LIBRARIES)
endif()

