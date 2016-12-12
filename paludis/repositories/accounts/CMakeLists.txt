
paludis_add_library(libpaludisaccountsrepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_exceptions.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_handler.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_repository_store.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_dep_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/accounts_installed_mask.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/installed_accounts_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/dummy_accounts_handler.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/passwd_accounts_handler.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisaccountsrepository libpaludisutil_SE)

paludis_add_test(accounts_repository GTEST)

