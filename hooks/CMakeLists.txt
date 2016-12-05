
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/eselect_env_update.bash.in"
               "${CMAKE_CURRENT_BINARY_DIR}/eselect_env_update.bash"
               @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/news.hook.in"
               "${CMAKE_CURRENT_BINARY_DIR}/news.hook"
               @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_install.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_install-hooks.cmake"
               @ONLY)

#paludis_add_test(eselect_env_update BASH
#                 TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_test.bash")
#paludis_add_test(news BASH
#                 TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/run_test.bash")

install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/gnu_info_index.bash"
          "${CMAKE_CURRENT_BINARY_DIR}/eselect_env_update.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/log.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/installable_cache_regen.bash"
          "${CMAKE_CURRENT_SOURCE_DIR}/installed_cache_regen.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/hooks/common")
install(PROGRAMS
          "${CMAKE_CURRENT_BINARY_DIR}/news.hook"
          "${CMAKE_CURRENT_SOURCE_DIR}/find_config_updates.hook"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/hooks/auto")
install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/update_config_protect_list.bash"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/hooks/install_post")

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/cmake_install-hooks.cmake")

