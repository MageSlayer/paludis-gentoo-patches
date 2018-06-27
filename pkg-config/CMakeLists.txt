
set(PALUDIS_PKG_CONFIG_CFLAGS)
set(PALUDIS_PKG_CONFIG_LIBS
    "-lpaludis_${PALUDIS_PKG_CONFIG_SLOT} -lpaludisutil_${PALUDIS_PKG_CONFIG_SLOT} -lpaludisargs_${PALUDIS_PKG_CONFIG_SLOT} ${CMAKE_THREAD_LIBS_INIT}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/paludis.pc.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/paludis.pc"
               @ONLY)

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/paludis.pc"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

