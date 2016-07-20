
include(CheckFunctionExists)
include(CMakeParseArguments)

macro(paludis_check_function_exists function variable)
  set(pcfe_options REQUIRED)
  set(pcfe_single_value_args)
  set(pcfe_multiple_value_args)

  cmake_parse_arguments(PCFE "${pcfe_options}" "${pcfe_single_value_args}" "${pcfe_multiple_value_args}" ${ARGN})

  check_function_exists(${function} ${variable})
  if(PCFE_REQUIRED AND NOT ${variable})
    message(SEND_ERROR "required function `${function}` not found")
  endif()
endmacro()

