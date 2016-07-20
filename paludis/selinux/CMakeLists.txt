
paludis_add_library(libpaludisselinux
                      "${CMAKE_CURRENT_SOURCE_DIR}/security_context.cc")
target_link_libraries(libpaludisselinux
                      PRIVATE
                        ${CMAKE_DL_LIBS})

install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/security_context.hh"
        DESTINATION
          "${CMAKE_INSTALL_FULL_INCLUDEDIR}/paludis-${PALUDIS_PKG_CONFIG_SLOT}/paludis/selinux")
install(TARGETS
          libpaludisselinux
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}")

