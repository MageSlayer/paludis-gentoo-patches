
paludis_add_library(libpaludistestenvironment
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/test_environment.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludistestenvironment libpaludis_SE libpaludisutil_SE)

install(FILES
          ${CMAKE_CURRENT_SOURCE_DIR}/test_environment.hh
        DESTINATION
          "${CMAKE_INSTALL_FULL_INCLUDEDIR}/paludis-${PALUDIS_PKG_CONFIG_SLOT}/paludis/environments/test")

