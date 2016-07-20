
if(ENABLE_VIM)
  install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/paludis.vim"
          DESTINATION
            "${PALUDIS_VIM_INSTALL_DIR}/ftdetect"
          COMPONENT
            vim)
endif()

