# vim: set et fdm=marker fmr={{{,}}} sw=2 sts=2 ts=8:

cmake_minimum_required(VERSION 3.5.0)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0051 NEW)
cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0058 NEW)

project(paludis VERSION 3.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(GNUInstallDirs)
include(CMakeDependentOption)

# TODO(compnerd) figure out if we want to just replace the HTMLDIR usage
set(CMAKE_INSTALL_FULL_HTMLDIR "${CMAKE_INSTALL_FULL_DOCDIR}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(PALUDIS_PKG_CONFIG_SLOT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

option(BUILD_SHARED_LIBS "build shared libraries" ON)
option(ENABLE_DOXYGEN "enable doxygen based documentation" OFF)
option(ENABLE_DOXYGEN_TAGS "use 'wget' to fetch external doxygen tags" OFF)
option(ENABLE_GTEST "enable GTest based tests" ON)
option(ENABLE_PBINS "enable pbins (nonfunctional, for development only)" OFF)
option(ENABLE_PYTHON "enable python interface (default: OFF)" OFF)
cmake_dependent_option(ENABLE_PYTHON_DOCS "build the python documentation" ON
                       "ENABLE_PYTHON" OFF)
option(ENABLE_RUBY "enable ruby interface (default: off)" OFF)
cmake_dependent_option(ENABLE_RUBY_DOCS "build the ruby documentation" ON
                       "ENABLE_RUBY" OFF)
option(ENABLE_SEARCH_INDEX "enable search index (requires sqlite3)" OFF)
option(ENABLE_STRIPPER "build the stripper (requires libmagic)" ON)
option(ENABLE_VIM "whether to install vim scripts" OFF)
option(ENABLE_XML "enable xml support for metadata.xml and GLSA support" OFF)
option(PALUDIS_COLOUR_PINK "use the pink colourscheme" OFF)
if(CMAKE_CROSSCOMPILING)
  option(USE_PREBUILT_DOCUMENTATION "use prebuilt documentation" ON)
else()
  option(USE_PREBUILT_DOCUMENTATION "use prebuilt documentation" OFF)
endif()

set(PALUDIS_VIM_INSTALL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/vim/vimfiles" CACHE PATH
    "vim installation directory")

set(PYTHON_SUPPORTED_VERSIONS 2.7 3.5 3.6 3.7 3.8)
set(PYTHON_VERSION "2.7" CACHE STRING
  "Python version (${PYTHON_SUPPORTED_VERSIONS}) (requires corresponding Boost.Python as well)")

set(RUBY_SUPPORTED_VERSIONS 2.4 2.5 2.6 2.7)
set(RUBY_VERSION "2.7" CACHE STRING "Ruby Version (${RUBY_SUPPORTED_VERSIONS})")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

include(PaludisList)
include(PaludisAddLibrary)
include(PaludisAddTest)
include(PaludisCheckFunctionExists)
include(PaludisCompileFlags)

# {{{ __cxa_demangle
CHECK_INCLUDE_FILE_CXX(cxxabi.h HAVE_CXXABI_H)
if(HAVE_CXXABI_H)
  CHECK_CXX_SOURCE_COMPILES("
    #include <typeinfo>
    #include <cxxabi.h>
    #include <stdlib.h>
    int main(void) {
      const char *name = typeid(name).name();
      int status;
      char *demangled = abi::__cxa_demangle(name, 0, 0, &status);
      free(demangled);
      return 0;
    }
  "
  HAVE_CXA_DEMANGLE)
endif()
# }}}

# {{{ miscellaneous functions
paludis_check_function_exists(strerror HAVE_STRERROR REQUIRED)
paludis_check_function_exists(signal HAVE_SIGNAL REQUIRED)
paludis_check_function_exists(grantpt HAVE_GRANTPT REQUIRED)
paludis_check_function_exists(unlockpt HAVE_UNLOCKPT REQUIRED)
paludis_check_function_exists(pipe2 HAVE_PIPE2 REQUIRED)

paludis_check_function_exists(ptsname_r HAVE_PTSNAME_R)
paludis_check_function_exists(ptsname HAVE_PTSNAME)
if(NOT HAVE_PTSNAME_R AND NOT HAVE_PTSNAME)
  message(SEND_ERROR "required function `ptsname_r` or `ptsname` not found")
endif()

paludis_check_function_exists(lchflags HAVE_LCHFLAGS)
paludis_check_function_exists(utimensat HAVE_UTIMENSAT)

paludis_check_function_exists(canonicalize_file_name HAVE_CANONICALIZE_FILE_NAME)
# }}}

# {{{ f*xattr family
CHECK_INCLUDE_FILE(sys/xattr.h HAVE_SYS_XATTR_H)
if(HAVE_SYS_XATTR_H)
  CHECK_C_SOURCE_COMPILES("
    #include <sys/xattr.h>
    int main(void) {
      flistxattr(0, 0, 0);
      fgetxattr(0, 0, 0, 0);
      fsetxattr(0, 0, 0, 0, 0);
      return 0;
    }
  "
  HAVE_XATTRS)
endif()
# }}}

# {{{ dirent.d_type
CHECK_C_SOURCE_COMPILES("
  #include <sys/types.h>
  #include <dirent.h>
  int main(void) {
    struct dirent dent;
    dent.d_type = DT_LINK;
    return 0;
  }
"
HAVE_DIRENT_DTYPE)
# }}}

# {{{ fallocate
CHECK_C_SOURCE_COMPILES("
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/types.h>
  #include <linux/falloc.h>
  int main(void) {
    return fallocate(0, FALLOC_FL_KEEP_SIZE, 0, 100);
  }
"
HAVE_FALLOCATE)
# }}}

# TODO(compnerd) find_library(RT_LIBRARY NAMES rt)

# {{{ -O3/extern template failure
CHECK_CXX_SOURCE_COMPILES("
template <typename>
struct F;

template <typename T_>
struct T {
  void f();
};

template <typename T_>
void T<T_>::f() { F<T_>::g(); }

extern template class T<int>;

int main(void) {
  T<int> f;
  f.f();
  return 0;
}
"
CXX_INLINES_EXTERN_TEMPLATE)
if(CXX_INLINES_EXTERN_TEMPLATE)
  message(SEND_ERROR "${CMAKE_CXX} inlines extern template (GCC bug 39242?); try -O2 or -fno-inline-functions")
endif()
# }}}

# {{{ explicitly instantiate fully specialized templates
CHECK_CXX_SOURCE_COMPILES("
  template <typename T_>
  struct S;

  template <>
  struct S<int> { };

  template class S<int>;
"
CXX_NEEDS_EXPLICIT_INSTANTIATION)
if(NOT CXX_NEEDS_EXPLICIT_INSTANTIATION)
  add_definitions(-DPALUDIS_NO_EXPLICIT_FULLY_SPECIALISED=1)
endif()
# }}}

# {{{ paludis repositories
set(PALUDIS_ALL_REPOSITORIES
    accounts;e;fake;gemcutter;repository;unavailable;unpackaged;unwritten)
set(PALUDIS_DEFAULT_REPOSITORIES
    e;fake;repository;unavailable;unpackaged;unwritten)
set(PALUDIS_REPOSITORIES ${PALUDIS_DEFAULT_REPOSITORIES} CACHE STRING
    "paludis repositories (all;default;${PALUDIS_ALL_REPOSITORIES})")

list_replace(PALUDIS_REPOSITORIES default "${PALUDIS_DEFAULT_REPOSITORIES}")
list_replace(PALUDIS_REPOSITORIES all "${PALUDIS_ALL_REPOSITORIES}")

set(REPOSITORY_GROUPS_DECLS)
foreach(repository ${PALUDIS_ALL_REPOSITORIES})
  string(TOUPPER ${repository} repository_uppercase)
  set(ENABLE_${repository_uppercase}_REPOSITORY FALSE)

  if(${repository} IN_LIST PALUDIS_REPOSITORIES)
    set(REPOSITORY_GROUP_IF_${repository} ${repository})
    set(ENABLE_${repository_uppercase}_REPOSITORY TRUE)
  endif()
  set(REPOSITORY_GROUPS_DECLS "${REPOSITORY_GROUPS_DECLS} struct ${repository};")
endforeach()
set(REPOSITORY_GROUPS_DECLS "${REPOSITORY_GROUPS_DECLS} struct semicolon_goes_where { }")
# }}}

# {{{ paludis environments
set(PALUDIS_ALL_ENVIRONMENTS
    paludis;portage;test)
set(PALUDIS_DEFAULT_ENVIRONMENTS
    paludis;test)
set(PALUDIS_ENVIRONMENTS "${PALUDIS_DEFAULT_ENVIRONMENTS}" CACHE STRING
    "paludis environments (all;default;${PALUDIS_ALL_ENVIRONMENTS})")

list_replace(PALUDIS_ENVIRONMENTS default "${PALUDIS_DEFAULT_ENVIRONMENTS}")
list_replace(PALUDIS_ENVIRONMENTS all "${PALUDIS_ALL_ENVIRONMENTS}")

set(ENVIRONMENT_GROUPS_DECLS)
foreach(environment ${PALUDIS_ALL_ENVIRONMENTS})
  if(${environment} IN_LIST PALUDIS_ENVIRONMENTS)
    set(ENVIRONMENT_GROUP_IF_${environment} environment_groups::${environment})

    string(TOUPPER ${environment} uc_environment)
    set(ENABLE_${uc_environment}_ENVIRONMENT 1)
  endif()
  set(ENVIRONMENT_GROUPS_DECLS "${ENVIRONMENT_GROUPS_DECLS} struct ${environment};")
endforeach()
set(ENVIRONMENT_GROUPS_DECLS "${ENVIRONMENT_GROUPS_DECLS} struct semicolon_goes_where { }")
# }}}

# {{{ paludis distribution
set(PALUDIS_DEFAULT_DISTRIBUTION gentoo CACHE STRING
    "paludis distribution (gentoo;exherbo)")
# }}}

# {{{ paludis clients
set(PALUDIS_ALL_CLIENTS
    cave)
set(PALUDIS_DEFAULT_CLIENTS
    cave)
set(PALUDIS_CLIENTS ${PALUDIS_DEFAULT_CLIENTS} CACHE STRING
    "paludis clients (all;default;${PALUDIS_ALL_CLIENTS})")

list_replace(PALUDIS_CLIENTS default ${PALUDIS_DEFAULT_CLIENTS})
list_replace(PALUDIS_CLIENTS all ${PALUDIS_ALL_CLIENTS})
# }}}

# {{{ configuration framework
set(CONFIG_FRAMEWORK auto CACHE STRING
    "paludis environments (auto;eselect;eclectic)")
if(${CONFIG_FRAMEWORK} STREQUAL "auto")
  execute_process(COMMAND
                    which eclectic
                  OUTPUT_VARIABLE
                    eclectic_path
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if("${eclectic_path}" STREQUAL "")
    set(CONFIG_FRAMEWORK eselect)
  else()
    set(CONFIG_FRAMEWORK eclectic)
  endif()
endif()
execute_process(COMMAND
                  ${CONFIG_FRAMEWORK} version
                RESULT_VARIABLE
                  exit_code
                OUTPUT_QUIET
                ERROR_QUIET)
if(NOT ${exit_code} EQUAL 0)
  message(SEND_ERROR "${CONFIG_FRAMEWORK} is required")
endif()

foreach(module env;news)
  execute_process(COMMAND
                    ${CONFIG_FRAMEWORK} ${module}
                  RESULT_VARIABLE
                    exit_code
                  OUTPUT_QUIET
                  ERROR_QUIET)
  if(NOT ${exit_code} EQUAL 0)
    message(SEND_ERROR "${CONFIG_FRAMEWORK} is missing the `${module}` module")
  endif()
endforeach()
# }}}

if(ENABLE_DOXYGEN)
  find_package(Doxygen REQUIRED)
endif()

if(ENABLE_GEMCUTTER_REPOSITORY)
  find_package(Jansson REQUIRED)
endif()

if(ENABLE_PBINS)
  find_package(LibArchive 3.0.4 REQUIRED)
endif()

if(ENABLE_PYTHON)
  list(FIND PYTHON_SUPPORTED_VERSIONS "${PYTHON_VERSION}" _python_version_index)
  if(_python_version_index EQUAL -1)
    message(SEND_ERROR "Invalid Python version specified (${PYTHON_VERSION})")
  endif()

  find_package(PythonInterp ${PYTHON_VERSION})
  if(NOT PYTHONINTERP_FOUND)
    message(SEND_ERROR "python is required for python support")
  endif()
  execute_process(COMMAND
                    "${PYTHON_EXECUTABLE}" -c "from distutils import sysconfig; print(sysconfig.get_python_lib(1))"
                  OUTPUT_VARIABLE
                    PALUDIS_PYEXECDIR
                  ERROR_QUIET
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  get_filename_component(PALUDIS_PYEXECDIR ${PALUDIS_PYEXECDIR} REALPATH)

  find_package(PythonLibs ${PYTHON_VERSION})

  find_package(Boost 1.67.0 REQUIRED
               COMPONENTS
                 python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})

  set(PALUDIS_PYTHON_INSTALL_DIR "${PALUDIS_PYEXECDIR}" CACHE PATH
      "python installation dir")
endif()

if(ENABLE_PYTHON_DOCS)
  if(NOT ENABLE_PYTHON)
    message(SEND_ERROR "python docs requires python to build")
  endif()

  find_package(Sphinx REQUIRED)
endif()

if(ENABLE_RUBY)
  list(FIND RUBY_SUPPORTED_VERSIONS "${RUBY_VERSION}" _ruby_version_index)

  if(_ruby_version_index EQUAL -1)
    message(SEND_ERROR "invalid ruby version specified (${RUBY_VERSION})")
  endif()

  find_package(Ruby ${RUBY_VERSION} REQUIRED)
endif()

set(PALUDIS_RUBY_INSTALL_DIR "${RUBY_SITEARCH_DIR}" CACHE PATH
    "ruby installation directory")

if(ENABLE_RUBY_DOCS)
  if(NOT ENABLE_RUBY)
    message(SEND_ERROR "ruby docs requires ruby to build")
  endif()

  execute_process(COMMAND
                    "${RUBY_EXECUTABLE}" -rubygems -e "require 'syntax/convertors/html'"
                  OUTPUT_VARIABLE
                    ruby_gem_syntax_available
                  OUTPUT_QUIET
                  ERROR_QUIET)
  if(NOT ${ruby_gem_syntax_available} EQUAL 0)
    message(SEND_ERROR "syntax (http://syntax.rubyforge.org) is needed to build ruby documentation")
  endif()
endif()

if(ENABLE_SEARCH_INDEX)
  find_package(SQLite3 REQUIRED)
endif()

if(ENABLE_STRIPPER)
  find_package(LibMagic REQUIRED)
endif()

if(ENABLE_XML)
  find_package(LibXml2 2.6 REQUIRED)
endif()

if(NOT USE_PREBUILT_DOCUMENTATION)
  find_program(ASCIIDOC_EXECUTABLE asciidoc)
  if(NOT ASCIIDOC_EXECUTABLE)
    message(SEND_ERROR "asciidoc is required for documentation")
  endif()

  find_program(XMLTO_EXECUTABLE xmlto)
  if(NOT XMLTO_EXECUTABLE)
    message(SEND_ERROR "xmlto is required for documentation")
  endif()

  find_program(TIDY_EXECUTABLE tidy)
  if(NOT TIDY_EXECUTABLE)
    message(SEND_ERROR "tidy is required for documentation")
  endif()
endif()

if(cave IN_LIST PALUDIS_CLIENTS)
  find_package(PCRECPP 7.8 REQUIRED)
endif()

enable_testing()
if(ENABLE_GTEST)
  find_package(GTest REQUIRED)
endif()

find_program(BASH_EXECUTABLE bash)
find_program(CAT_EXECUTABLE cat)
find_program(GIT_EXECUTABLE git)
find_program(SED_EXECUTABLE sed gsed)

configure_file("${PROJECT_SOURCE_DIR}/cmake/config.h.in"
               "${CMAKE_BINARY_DIR}/config.h")

add_definitions(-DSYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}"
                -DLIBEXECDIR="${CMAKE_INSTALL_FULL_LIBEXECDIR}"
                -DDATADIR="${CMAKE_INSTALL_FULL_DATADIR}"
                -DLIBDIR="${CMAKE_INSTALL_FULL_LIBDIR}")

include_directories("${CMAKE_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(misc)
add_subdirectory(paludis)
add_subdirectory(python)
add_subdirectory(ruby)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(hooks)
add_subdirectory(vim)
add_subdirectory(bash-completion)
add_subdirectory(zsh-completion)
add_subdirectory(pkg-config)

include(PaludisPackage)

