install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/src_prepare.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/list_functions.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/8")

