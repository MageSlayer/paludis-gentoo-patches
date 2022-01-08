
if(ENABLE_PYTHON_DOCS)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
                 @ONLY)

  set(PYTHON_EXAMPLES
      "${CMAKE_CURRENT_SOURCE_DIR}/example_about.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/example_command_line.py"
      "${CMAKE_CURRENT_SOURCE_DIR}/example_version_spec.py")

  set(RST_DOCS
      "${CMAKE_CURRENT_SOURCE_DIR}/example_about.rst"
      "${CMAKE_CURRENT_SOURCE_DIR}/example_command_line.rst"
      "${CMAKE_CURRENT_SOURCE_DIR}/example_version_spec.rst"
      "${CMAKE_CURRENT_SOURCE_DIR}/index.rst"
      "${CMAKE_CURRENT_SOURCE_DIR}/paludis.rst")

  add_custom_command(COMMAND
                       env PYTHONPATH="${CMAKE_BINARY_DIR}/python"
                       "${Python_EXECUTABLE}" "${SPHINX_EXECUTABLE}"
                         -c "${CMAKE_CURRENT_BINARY_DIR}"
                         -b html
                         -d "${CMAKE_CURRENT_BINARY_DIR}/doctrees"
                         "${CMAKE_CURRENT_SOURCE_DIR}"
                         "${CMAKE_CURRENT_BINARY_DIR}/python"
                     OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/python"
                     DEPENDS
                       "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
                       paludis
                       ${PYTHON_EXAMPLES}
                       ${RST_DOCS})
  add_custom_target(python-api-docs
                    ALL
                    DEPENDS
                      "${CMAKE_CURRENT_BINARY_DIR}/python")

  install(DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/python"
          DESTINATION
            "${CMAKE_INSTALL_FULL_HTMLDIR}/api")
endif()

