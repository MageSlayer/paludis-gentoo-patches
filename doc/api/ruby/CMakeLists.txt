
if(ENABLE_RUBY_DOCS)
  set(RUBY_EXAMPLES
      example_about.rb
      example_action.rb
      example_command_line.rb
      example_contents.rb
      example_dep_tree.rb
      example_environment.rb
      example_package_id.rb
      example_mask.rb
      example_match_package.rb
      example_repository.rb
      example_selection.rb
      example_version_operator.rb
      example_version_spec.rb)

  add_custom_command(COMMAND
                       env TOP_SRCDIR="${CMAKE_SOURCE_DIR}" "${RUBY_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/create_ruby_doc.rb" -t "Paludis Ruby API" -m Paludis --op "${CMAKE_CURRENT_BINARY_DIR}/ruby" "${CMAKE_SOURCE_DIR}/ruby/*.cc"
                     COMMAND
                       "${RUBY_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/create_ruby_examples.rb" "${CMAKE_CURRENT_SOURCE_DIR}" ${RUBY_EXAMPLES}
                     COMMAND
                      "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/ruby_syntax.css" "${CMAKE_CURRENT_BINARY_DIR}/ruby"
                     OUTPUT
                      "${CMAKE_CURRENT_BINARY_DIR}/ruby")
  add_custom_target(ruby-docs
                    ALL
                    DEPENDS
                      "${CMAKE_CURRENT_BINARY_DIR}/ruby")

  install(DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/ruby"
          DESTINATION
            "${CMAKE_INSTALL_FULL_HTMLDIR}/api")
endif()

