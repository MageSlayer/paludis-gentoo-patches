
if(ENABLE_VIM)
  install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-general-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-keywords-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-licenses-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-mirrors-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-output-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-package-mask-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-repositories-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-suggestions-conf.vim"
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis-use-conf.vim"
          DESTINATION
            "${PALUDIS_VIM_INSTALL_DIR}/syntax"
          COMPONENT
            vim)
endif()

