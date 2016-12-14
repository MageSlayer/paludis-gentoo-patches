
if(ENABLE_RUBY)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  paludis_add_library(libpaludisruby
                        "${CMAKE_CURRENT_SOURCE_DIR}/action.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/choice.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/contents.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/dep_label.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/dep_spec.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/environment.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/filter.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/filtered_generator.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/generator.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/log.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/mask.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/metadata_key.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/name.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/package_id.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/paludis_ruby.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/qualified_package_name.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/repository.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/selection.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/version_spec.cc"
                      NN_SOURCES
                        "${CMAKE_CURRENT_SOURCE_DIR}/nice_names.nn")
  target_include_directories(libpaludisruby SYSTEM
                             PRIVATE
                               ${RUBY_INCLUDE_DIRS})
  target_link_libraries(libpaludisruby
                        PRIVATE
                          libpaludis
                          libpaludisutil
                          ${RUBY_LIBRARY}
                          Threads::Threads)

  paludis_add_library(Paludis UNVERSIONED SHARED_LIBRARY
                        "${CMAKE_CURRENT_SOURCE_DIR}/paludis_ruby_so.cc")
  target_include_directories(Paludis SYSTEM
                             PRIVATE
                               ${RUBY_INCLUDE_DIRS})
  target_link_libraries(Paludis
                        PRIVATE
                          libpaludisruby)
  set_target_properties(Paludis PROPERTIES
                        OUTPUT_NAME
                          Paludis
                        PREFIX
                          "")

  foreach(test
            contents
            filter
            filtered_generator
            log
            qualified_package_name
            selection
            version_spec)
    paludis_add_test(${test} RUBY)
  endforeach()

  foreach(test
            action
            choice
            dep_spec
            environment
            generator
            package_id
            paludis_ruby
            repository)
    paludis_add_test(${test} RUBY)
  endforeach()

  add_subdirectory(demos)

  install(TARGETS
            libpaludisruby
          DESTINATION
            "${CMAKE_INSTALL_FULL_LIBDIR}")
  install(TARGETS
            Paludis
          DESTINATION
            "${PALUDIS_RUBY_INSTALL_DIR}")
endif()

