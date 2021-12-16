
paludis_add_library(libpaludispaludisenvironment
                    OBJECT_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/bashable_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/extra_distribution_data.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/keywords_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/licenses_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/mirrors_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/output_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/package_mask_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/paludis_config.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/paludis_environment.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/suggestions_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/use_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/world.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/registration.cc")
target_compile_definitions(libpaludispaludisenvironment
                           PRIVATE
                             -DSHAREDIR="${CMAKE_INSTALL_FULL_DATAROOTDIR}")
add_dependencies(libpaludispaludisenvironment libpaludis_SE libpaludisutil_SE)

paludis_add_test(paludis_environment GTEST)
paludis_add_test(world GTEST)

install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/default_output.conf"
        DESTINATION
          "${CMAKE_INSTALL_FULL_DATAROOTDIR}/paludis/environments/paludis")

