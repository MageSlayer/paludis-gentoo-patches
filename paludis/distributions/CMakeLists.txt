
add_subdirectory(exherbo)
add_subdirectory(gentoo)

install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/exherbo.conf"
          "${CMAKE_CURRENT_SOURCE_DIR}/gentoo.conf"
        DESTINATION
          "${CMAKE_INSTALL_FULL_DATADIR}/paludis/distributions")

