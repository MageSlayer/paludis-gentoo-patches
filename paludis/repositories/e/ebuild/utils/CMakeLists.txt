
add_subdirectory(4)
add_subdirectory(5)
add_subdirectory(exheres-0)

foreach(prep all;allstrip;allman;allinfo;strip;man;info;docs;alldocs)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/prep.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/prep${prep}"
                 @ONLY)
endforeach()

if(ENABLE_PBINS)
  add_executable(unpaxinate
                   "${CMAKE_CURRENT_SOURCE_DIR}/unpaxinate.cc")
  target_link_libraries(unpaxinate
                        PRIVATE
                          LibArchive::LibArchive
                       )
endif()

add_executable(print_exports
                 "${CMAKE_CURRENT_SOURCE_DIR}/print_exports.cc")
add_executable(locked_pipe_command
                 "${CMAKE_CURRENT_SOURCE_DIR}/locked_pipe_command.cc")
add_executable(strip_tar_corruption
                 "${CMAKE_CURRENT_SOURCE_DIR}/strip_tar_corruption.cc")

paludis_add_test(wrapped_getfsize BASH
                 EBUILD_MODULE_SUFFIXES 0
                 TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_test.bash")

install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/canonicalise"
          "${CMAKE_CURRENT_SOURCE_DIR}/dobin"
          "${CMAKE_CURRENT_SOURCE_DIR}/doconfd"
          "${CMAKE_CURRENT_SOURCE_DIR}/dodir"
          "${CMAKE_CURRENT_SOURCE_DIR}/dodoc"
          "${CMAKE_CURRENT_SOURCE_DIR}/doenvd"
          "${CMAKE_CURRENT_SOURCE_DIR}/doexe"
          "${CMAKE_CURRENT_SOURCE_DIR}/dohard"
          "${CMAKE_CURRENT_SOURCE_DIR}/dohtml"
          "${CMAKE_CURRENT_SOURCE_DIR}/doinfo"
          "${CMAKE_CURRENT_SOURCE_DIR}/doinitd"
          "${CMAKE_CURRENT_SOURCE_DIR}/doins"
          "${CMAKE_CURRENT_SOURCE_DIR}/dolib"
          "${CMAKE_CURRENT_SOURCE_DIR}/dolib.a"
          "${CMAKE_CURRENT_SOURCE_DIR}/dolib.so"
          "${CMAKE_CURRENT_SOURCE_DIR}/doman"
          "${CMAKE_CURRENT_SOURCE_DIR}/domo"
          "${CMAKE_CURRENT_SOURCE_DIR}/donewins"
          "${CMAKE_CURRENT_SOURCE_DIR}/dosbin"
          "${CMAKE_CURRENT_SOURCE_DIR}/dosed"
          "${CMAKE_CURRENT_SOURCE_DIR}/dosym"
          "${CMAKE_CURRENT_SOURCE_DIR}/ecompress"
          "${CMAKE_CURRENT_SOURCE_DIR}/ecompressdir"
          "${CMAKE_CURRENT_SOURCE_DIR}/emake"
          "${CMAKE_CURRENT_SOURCE_DIR}/fowners"
          "${CMAKE_CURRENT_SOURCE_DIR}/fperms"
          "${CMAKE_CURRENT_SOURCE_DIR}/keepdir"
          "${CMAKE_CURRENT_SOURCE_DIR}/newbin"
          "${CMAKE_CURRENT_SOURCE_DIR}/newconfd"
          "${CMAKE_CURRENT_SOURCE_DIR}/newdoc"
          "${CMAKE_CURRENT_SOURCE_DIR}/newenvd"
          "${CMAKE_CURRENT_SOURCE_DIR}/newexe"
          "${CMAKE_CURRENT_SOURCE_DIR}/newinitd"
          "${CMAKE_CURRENT_SOURCE_DIR}/newins"
          "${CMAKE_CURRENT_SOURCE_DIR}/newlib.a"
          "${CMAKE_CURRENT_SOURCE_DIR}/newlib.so"
          "${CMAKE_CURRENT_SOURCE_DIR}/newman"
          "${CMAKE_CURRENT_SOURCE_DIR}/newsbin"
          "${CMAKE_CURRENT_BINARY_DIR}/prepall"
          "${CMAKE_CURRENT_BINARY_DIR}/prepallstrip"
          "${CMAKE_CURRENT_BINARY_DIR}/prepstrip"
          "${CMAKE_CURRENT_BINARY_DIR}/prepallman"
          "${CMAKE_CURRENT_BINARY_DIR}/prepman"
          "${CMAKE_CURRENT_BINARY_DIR}/prepallinfo"
          "${CMAKE_CURRENT_BINARY_DIR}/prepinfo"
          "${CMAKE_CURRENT_BINARY_DIR}/prepdocs"
          "${CMAKE_CURRENT_BINARY_DIR}/prepalldocs"
          "${CMAKE_CURRENT_SOURCE_DIR}/unpack"
          "${CMAKE_CURRENT_SOURCE_DIR}/wrapped_ldconfig"
          "${CMAKE_CURRENT_SOURCE_DIR}/wrapped_getfsize"
          "${CMAKE_CURRENT_SOURCE_DIR}/wrapped_getmtime"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils")
install(TARGETS
          print_exports
          locked_pipe_command
          strip_tar_corruption
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils")
if(ENABLE_PBINS)
  install(TARGETS
            unpaxinate
          DESTINATION
            "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/utils")
endif()

