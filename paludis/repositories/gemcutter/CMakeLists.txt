
paludis_add_library(libpaludisgemcutterrepository
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/gemcutter_dependencies_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/gemcutter_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/gemcutter_uri_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/gemcutter_repository.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/gemcutter_repository_store.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/json_common.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/json_things_handle.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
add_dependencies(libpaludisgemcutterrepository libpaludis_SE libpaludisutil_SE)

paludis_add_library(libpaludisgemcutterrepositoryjsonthings
                      json_things.cc)
target_link_libraries(libpaludisgemcutterrepositoryjsonthings
                      PRIVATE
                        Jansson::Jansson)

install(TARGETS
          libpaludisgemcutterrepositoryjsonthings
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}")
