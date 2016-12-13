# .rst
# FindJansson
# -----------
#
# Find Jansson library and headers
#
# The module defines the following variables:
#
# ::
#
#   Jansson_FOUND       - true if Jansson was found
#   Jansson_INCLUDE_DIR - include search path
#   Jansson_LIBRARIES   - libraries to link
#   Jansson_VERSION     - libmagic 3-component version number

if(Jansson_INCLUDE_DIRS AND Jansson_LIBRARIES)
  set(Jansson_FOUND TRUE)
else()
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_JANSSON QUIET jansson)

  find_path(Jansson_INCLUDE_DIR
            NAMES
              jansson.h
            HINTS
              ${PC_JANSSON_INCLUDEDIR}
              ${PC_JANSSON_INCLUDE_DIRS}
              ${CMAKE_INSTALL_FULL_INCLUDEDIR})
  find_library(Jansson_LIBRARIES
               NAMES
                 jansson
                 libjansson
               HINTS
                 ${PC_JANSSON_LIBDIR}
                 ${PC_JANSSON_LIBRARY_DIRS}
                 ${CMAKE_INSTALL_FULL_LIBDIR})

  if(Jansson_INCLUDE_DIR AND EXISTS "${Jansson_INCLUDE_DIR}/jansson.h")
    file(STRINGS "${Jansson_INCLUDE_DIR}/jansson.h" jansson_version_str
         REGEX "^#[ ]*define[ ]+JANSSON_VERSION[ ]+\".*\"")
    string(REGEX
           REPLACE "^[ ]*define[ ]+JANSSON_VERSION[ ]+\"([^\"]*)\".*" "\\1"
           Jansson_VERSION_STRING "${jansson_version_str}")
    unset(jansson_version_str)
  endif()

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Jansson
                                    REQUIRED_VARS
                                      Jansson_INCLUDE_DIR
                                      Jansson_LIBRARIES
                                    VERSION_VAR
                                      Jansson_VERSION_STRING)
  mark_as_advanced(Jansson_INCLUDE_DIR Jansson_LIBRARIES)
endif()

