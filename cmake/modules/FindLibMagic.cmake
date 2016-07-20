#.rst:
# FindLibMagic
# ------------
#
# Find libmagic library and headers
#
# The module defines the following variables:
#
# ::
#
#   LibMagic_FOUND       - true if libmagic was found
#   LibMagic_INCLUDE_DIR - include search path
#   LibMagic_LIBRARIES   - libraries to link

if(UNIX)
  find_path(LibMagic_INCLUDE_DIR
            NAMES
              magic.h)

  if(APPLE)
    set(LibMagic_NAMES libmagic.a magic)
  else()
    set(LibMagic_NAMES magic)
  endif()

  find_library(LibMagic_LIBRARIES
               NAMES
                 ${LibMagic_NAMES}
               HINTS
                 ${LIBMAGIC_ROOT_DIR}
                 ${CMAKE_INSTALL_PREFIX})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(LibMagic
                                    REQUIRED_VARS
                                      LibMagic_LIBRARIES
                                      LibMagic_INCLUDE_DIR)
  mark_as_advanced(LibMagic_LIBRARIES LibMagic_INCLUDE_DIR)
endif()

