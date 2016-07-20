
install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/build_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/list_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/multilib_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/output_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_install.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_prepare.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/6")

