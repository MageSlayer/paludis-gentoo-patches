
paludis_add_library(libpaludisfakerepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/dep_parser.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/fake_repository_base.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/fake_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/fake_installed_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/fake_package_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisfakerepository libpaludis_SE libpaludisutil_SE)

paludis_add_test(fake_repository GTEST)
paludis_add_test(fake_installed_repository GTEST)
# paludis_add_test(dep_parser)

install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/dep_parser.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/fake_repository.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/fake_repository_base.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/fake_installed_repository.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/fake_package_id.hh"
        DESTINATION
          "${CMAKE_INSTALL_FULL_INCLUDEDIR}/paludis-${PALUDIS_PKG_CONFIG_SLOT}/paludis/repositories/fake")

