
paludis_add_library(libpaludisunpackagedrepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/exceptions.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unpackaged_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unpackaged_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unpackaged_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unpackaged_stripper.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/installed_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/installed_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisunpackagedrepository libpaludisutil_SE)

paludis_add_test(unpackaged_repository GTEST)
paludis_add_test(installed_repository GTEST)

