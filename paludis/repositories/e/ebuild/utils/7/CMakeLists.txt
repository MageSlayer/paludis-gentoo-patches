
foreach(bannedscript
          dohtml
          dolib)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_7"
                 "${CMAKE_CURRENT_BINARY_DIR}/${bannedscript}"
                 @ONLY)
endforeach()

install(PROGRAMS
          "${CMAKE_CURRENT_BINARY_DIR}/dohtml"
          "${CMAKE_CURRENT_BINARY_DIR}/dolib"
          "${CMAKE_CURRENT_SOURCE_DIR}/dolib.a"
          "${CMAKE_CURRENT_SOURCE_DIR}/dolib.so"
          "${CMAKE_CURRENT_SOURCE_DIR}/banned_in_eapi_7"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils/7")

