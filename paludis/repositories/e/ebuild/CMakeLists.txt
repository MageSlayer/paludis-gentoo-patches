
add_subdirectory(0)
add_subdirectory(1)
add_subdirectory(2)
add_subdirectory(3)
add_subdirectory(4)
add_subdirectory(5)
add_subdirectory(6)
add_subdirectory(exheres-0)
add_subdirectory(paludis-1)
add_subdirectory(pbin-1)
add_subdirectory(utils)

paludis_add_test(kernel_functions BASH
                 EBUILD_MODULE_SUFFIXES 0
                 TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_test.bash")

install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/binary_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/die_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/ebuild.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/kernel_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/install_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/multilib_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/output_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/pipe_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/sandbox.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/sydbox.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/source_functions.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/usage_error.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/write_vdb_entry.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/write_binary_ebuild.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis")

