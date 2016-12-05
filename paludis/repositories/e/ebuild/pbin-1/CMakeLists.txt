
install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/builtin_installbin.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/builtin_pivotbin.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/pbin-1")

