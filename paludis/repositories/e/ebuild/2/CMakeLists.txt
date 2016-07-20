
install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/eclass_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/pkg_nofetch.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_compile.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_configure.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_install.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_prepare.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_test.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/src_unpack.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/output_functions.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/2")

