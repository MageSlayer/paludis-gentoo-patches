#.rst:
# FindSphinx
# ------------
#
# Find sphinx documentation generator
#
# The module defines the following variables:
#
# ::
#
#   SPHINX_FOUND      - true if sphinx was found
#   SPHINX_EXECUTABLE - sphinx binary

find_program(SPHINX_EXECUTABLE
             NAMES
               sphinx-build
             HINTS
               ${SPHINX_ROOT_DIR}
               ${CMAKE_INSTALL_PREFIX})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)
mark_as_advanced(SPHINX_EXECUTABLE)

