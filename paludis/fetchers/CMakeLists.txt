
foreach(scheme http;https;ftp)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dowget.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()

add_custom_command(OUTPUT
                     "${CMAKE_CURRENT_BINARY_DIR}/.keep"
                   COMMAND
                     "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/.keep")
# FIXME(compnerd) auto-generate the target
add_custom_target(fetchers-keep
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/.keep")

add_subdirectory(demos)

install(PROGRAMS
          "${CMAKE_CURRENT_BINARY_DIR}/dohttp"
          "${CMAKE_CURRENT_BINARY_DIR}/dohttps"
          "${CMAKE_CURRENT_BINARY_DIR}/doftp"
          "${CMAKE_CURRENT_SOURCE_DIR}/dofile"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/fetchers")
install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/.keep"
        DESTINATION
          "${CMAKE_INSTALL_FULL_DATADIR}/paludis/fetchers")

