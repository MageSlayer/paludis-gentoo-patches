
install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/builtin_bad_required_use.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/pkg_pretend.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_install.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/die_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/output_functions.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/4")

