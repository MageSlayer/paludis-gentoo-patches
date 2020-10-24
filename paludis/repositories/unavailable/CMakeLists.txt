
paludis_add_library(libpaludisunavailablerepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_package_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_repository_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_repository_dependencies_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_mask.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_repository_store.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unavailable_repository_file.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisunavailablerepository libpaludis_SE libpaludisutil_SE)

paludis_add_test(unavailable_repository GTEST)

