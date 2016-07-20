#.rst:
# FindPCRECPP
# ------------
#
# Find libpcrecpp library and headers
#
# The module defines the following variables:
#
# ::
#
#   PCRECPP_FOUND       - true if libmagic was found
#   PCRECPP_INCLUDE_DIR - include search path
#   PCRECPP_LIBRARIES   - libraries to link

find_path(PCRECPP_INCLUDE_DIR
          NAMES
            pcrecpp.h)

find_library(PCRECPP_LIBRARY
             NAMES
               pcrecpp
             HINTS
               ${PCRECPP_ROOT_DIR}
               ${CMAKE_INSTALL_PREFIX})
find_library(PCRE_LIBRARY
             NAMES
               pcre
             HINTS
               ${PCRECPP_ROOT_DIR}
               ${CMAKE_INSTALL_PREFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRECPP
                                  REQUIRED_VARS
                                    PCRECPP_LIBRARY
                                    PCRE_LIBRARY
                                    PCRECPP_INCLUDE_DIR)
if(PCRECPP_FOUND)
  set(PCRECPP_LIBRARIES ${PCRECPP_LIBRARY} ${PCRE_LIBRARY})
endif()
mark_as_advanced(PCRECPP_INCLUDE_DIR PCRE_LIBRARY PCRECPP_LIBRARY PCRECPP_LIBRARIES)

