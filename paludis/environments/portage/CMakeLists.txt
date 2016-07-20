
paludis_add_library(libpaludisportageenvironment
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/portage_environment.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")

paludis_add_test(portage_environment GTEST)

install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/bashrc"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/environments/portage")

