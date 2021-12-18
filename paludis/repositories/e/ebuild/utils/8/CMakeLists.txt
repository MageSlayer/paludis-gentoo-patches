foreach(bannedscript
          hasq
          hasv
          useq)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_8"
                 "${CMAKE_CURRENT_BINARY_DIR}/${bannedscript}"
                 @ONLY)
endforeach()

install(PROGRAMS
          "${CMAKE_CURRENT_BINARY_DIR}/hasq"
          "${CMAKE_CURRENT_BINARY_DIR}/hasv"
          "${CMAKE_CURRENT_BINARY_DIR}/useq"
          "${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_8"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils/8")
