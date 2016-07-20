
paludis_add_library(libpaludisunwrittenrepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/unwritten_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unwritten_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unwritten_mask.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unwritten_repository_store.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/unwritten_repository_file.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisunwrittenrepository libpaludisutil_SE)

paludis_add_test(unwritten_repository GTEST)

