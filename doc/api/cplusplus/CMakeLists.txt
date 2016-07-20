
add_subdirectory(examples)

set(DOXYGEN_TAG_FILE)
if(ENABLE_DOXYGEN_TAGS)
  set(DOXYGEN_TAG_FILE "${CMAKE_CURRENT_BINARY_DIR}/libstdc++.tag")
  file(DOWNLOAD
       "http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag"
       "${DOXYGEN_TAG_FILE}")
endif()

if(ENABLE_DOXYGEN)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf"
                 @ONLY)

  add_custom_command(COMMAND
                      "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf"
                     DEPENDS
                       "${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf"
                       "${CMAKE_CURRENT_SOURCE_DIR}/groups.doxygen"
                       "${CMAKE_CURRENT_SOURCE_DIR}/main_page.doxygen"
                       "${CMAKE_CURRENT_SOURCE_DIR}/namespaces.doxygen"
                       "${CMAKE_CURRENT_SOURCE_DIR}/references.doxygen"
                       "${DOXYGEN_TAG_FILE}"
                     OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/cplusplus")
  add_custom_target(c++-api-docs
                    ALL
                    DEPENDS
                      "${CMAKE_CURRENT_BINARY_DIR}/cplusplus")

  install(DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/cplusplus"
          DESTINATION
            "${CMAKE_INSTALL_FULL_HTMLDIR}/api")
endif()

