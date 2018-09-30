
install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/install_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/version_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/builtin_init.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/7")

