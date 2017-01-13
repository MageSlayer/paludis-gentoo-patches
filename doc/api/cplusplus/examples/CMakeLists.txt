
# TODO(compnerd) populate PROJECT_VERSION_TWEAK and PALUDIS_GIT_HEAD
add_definitions(-DPALUDIS_PACKAGE="${PROJECT_NAME}"
                -DPALUDIS_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
                -DPALUDIS_VERSION_MINOR=${PROJECT_VERSION_MINOR}
                -DPALUDIS_VERSION_MICRO=${PROJECT_VERSION_PATCH}
                -DPALUDIS_VERSION_SUFFIX=""
                -DPALUDIS_GIT_HEAD="")

paludis_add_library(libpaludisexamples
                    STATIC_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/example_command_line.cc")
target_link_libraries(libpaludisexamples
                      INTERFACE
                        libpaludisargs)
add_dependencies(libpaludisexamples libpaludisargs_SE)

function(add_example example)
  add_executable(example_${example}
                   "${CMAKE_CURRENT_SOURCE_DIR}/example_${example}.cc")
  target_link_libraries(example_${example}
                        PRIVATE
                          libpaludisexamples
                          libpaludis
                          libpaludisutil)
endfunction()

foreach(example
          about
          action
          contents
          dep_label
          dep_spec
          dep_spec_flattener
          environment
          package_id
          mask
          metadata_key
          repository
          match_package
          selection
          version_operator
          version_spec
          name)
  add_example(${example})
endforeach()

