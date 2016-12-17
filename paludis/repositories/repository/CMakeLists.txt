
paludis_add_library(libpaludisrepositoryrepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/repository_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/repository_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/repository_repository_store.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisrepositoryrepository libpaludisutil_SE)

