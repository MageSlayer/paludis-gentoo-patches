
foreach(bannedscript
          dohard
          dosed)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_4"
                 "${CMAKE_CURRENT_BINARY_DIR}/${bannedscript}"
                 @ONLY)
endforeach()

install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/docompress"
          "${CMAKE_CURRENT_BINARY_DIR}/dohard"
          "${CMAKE_CURRENT_BINARY_DIR}/dosed"
          "${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_4"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils/4")

