
install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/e.conf"
          "${CMAKE_CURRENT_SOURCE_DIR}/gems.conf"
          "${CMAKE_CURRENT_SOURCE_DIR}/paludis.conf"
          "${CMAKE_CURRENT_SOURCE_DIR}/repository_blacklist.conf"
        DESTINATION
          "${CMAKE_INSTALL_FULL_DATADIR}/paludis/distributions/gentoo")

