
if(CXX_SUPPORTS_FNO_STRICT_ALIASING)
  add_compile_options(-fno-strict-aliasing)
endif()

if(ENABLE_PYTHON)
  paludis_add_library(libpaludispython
                        "${CMAKE_CURRENT_SOURCE_DIR}/about.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/action.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/choices.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/contents.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/dep_label.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/dep_spec.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/environment.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/exception.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/filter.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/filtered_generator.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/fs_path.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/generator.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/mask.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/match_package.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/metadata_key.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/mutex.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/name.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/log.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/package_id.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/repository.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/selection.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/slot_requirement.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/version_operator.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/version_requirements.cc"
                        "${CMAKE_CURRENT_SOURCE_DIR}/version_spec.cc"
                      NN_SOURCES
                        "${CMAKE_CURRENT_SOURCE_DIR}/nice_names.nn")
  if(CXX_SUPPORTS_G0)
    target_compile_options(libpaludispython
                           PRIVATE
                             -g0)
  endif()
  target_include_directories(libpaludispython
                             PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               ${Boost_INCLUDE_DIRS})
  target_link_libraries(libpaludispython
                        PRIVATE
                          libpaludis)

  paludis_add_library(paludis UNVERSIONED SHARED_LIBRARY
                        "${CMAKE_CURRENT_SOURCE_DIR}/paludis_python_so.cc")
  target_include_directories(paludis
                             PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               ${Boost_INCLUDE_DIRS})
  target_link_libraries(paludis
                        PRIVATE
                          ${Boost_LIBRARIES}
                          ${PYTHON_LIBRARIES}
                          libpaludispython)
  set_target_properties(paludis PROPERTIES
                        OUTPUT_NAME
                          paludis
                        PREFIX
                          "")

  paludis_add_library(libadditionaltests
                        "${CMAKE_CURRENT_SOURCE_DIR}/additional_tests.cc")
  target_include_directories(libadditionaltests
                             PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               ${Boost_INCLUDE_DIRS})
  target_link_libraries(libadditionaltests
                        PRIVATE
                          libpaludispython)

  paludis_add_library(additional_tests UNVERSIONED SHARED_LIBRARY
                        "${CMAKE_CURRENT_SOURCE_DIR}/additional_tests_so.cc")
  target_include_directories(additional_tests
                             PRIVATE
                               ${PYTHON_INCLUDE_DIRS}
                               ${Boost_INCLUDE_DIRS})
  target_link_libraries(additional_tests
                        PRIVATE
                          ${Boost_PYTHON_LIBRARY}
                          libadditionaltests)
  set_target_properties(additional_tests PROPERTIES
                        OUTPUT_NAME
                          additional_Tests
                        PREFIX
                          "")

  foreach(test
            action
            contents
            dep_label
            dep_spec
            filter
            filtered_generator
            generator
            log
            name
            selection
            version_operator
            version_requirements
            version_spec)
    paludis_add_test(${test} PYTHON)
  endforeach()

  foreach(test
            choices
            environment
            mask
            metadata_key
            package_id
            repository)
    paludis_add_test(${test} PYTHON)
  endforeach()

  install(TARGETS
            libpaludispython
          DESTINATION
            "${CMAKE_INSTALL_FULL_LIBDIR}")
  install(TARGETS
            paludis
          DESTINATION
            "${PALUDIS_PYTHON_INSTALL_DIR}")
  install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis_output_wrapper.py"
          DESTINATION
            "${PALUDIS_PYTHON_INSTALL_DIR}")
endif()

