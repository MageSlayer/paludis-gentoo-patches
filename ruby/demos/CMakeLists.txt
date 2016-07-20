
if(ENABLE_RUBY)
  install(CODE
            "
            execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink \"${CMAKE_INSTALL_FULL_DATADIR}/paludis/ruby/demos/playman.rb\" \"${CMAKE_CURRENT_BINARY_DIR}/playman\")
            ")
  install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/version_spec_distributions.rb"
            "${CMAKE_CURRENT_SOURCE_DIR}/latest_stable.rb"
            "${CMAKE_CURRENT_SOURCE_DIR}/playman.rb"
          DESTINATION
            "${CMAKE_INSTALL_FULL_DATADIR}/paludis/ruby/demos")
  install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/playman"
          DESTINATION
            "${CMAKE_INSTALL_FULL_BINDIR}")
endif()

