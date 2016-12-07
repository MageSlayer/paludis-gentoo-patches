
include(CMakeParseArguments)

find_program(M4_EXECUTABLE NAMES m4 gm4 DOC "m4 macro processor")
if(NOT M4_EXECUTABLE)
  message(FATAL_ERROR "m4 not found")
endif()

function(paludis_m4process input_file target)
  set(options)
  set(single_value_args OUTPUT)
  set(multiple_value_args)

  cmake_parse_arguments(PM4P "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  get_filename_component(input_file_basename "${input_file}" NAME)
  if(PM4P_OUTPUT)
    set(output_file "${PM4P_OUTPUT}")
  else()
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/${input_file_basename}")
    string(REGEX REPLACE "\\.[^.]*$" "" output_file "${output_file}")
  endif()

  string(MD5 md5 "paludis-m4process-${output_file}")
  set(target_name paludis-m4process-${md5}-${input_file_basename})

  add_custom_command(OUTPUT
                       ${output_file}
                     COMMAND
                       ${M4_EXECUTABLE} -I ${PROJECT_SOURCE_DIR} -E ${input_file} > ${output_file}
                     DEPENDS
                       ${input_file})
  add_custom_target(${target_name} DEPENDS ${output_file} COMMENT ${output_file})

  set(${target} ${target_name} PARENT_SCOPE)
endfunction()

# TODO(compnerd) convert misc/make_se.bash into a cmake script
function(paludis_seprocess input_file)
  set(single_value_args HEADER_TARGET SOURCE_TARGET)
  set(multiple_value_args)

  cmake_parse_arguments(PSEP "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  if(NOT PSEP_HEADER_TARGET)
    message(SEND_ERROR "paludis_seprocess(${input_file}) invoked without HEADER_TARGET option")
  endif()
  if(NOT PSEP_SOURCE_TARGET)
    message(SEND_ERROR "paludis_seprocess(${input_file}) invoked without SOURCE_TARGET option")
  endif()

  get_filename_component(input_file_basename "${input_file}" NAME)
  get_filename_component(input_file_basename_we "${input_file}" NAME_WE)

  set(output_header_file "${CMAKE_CURRENT_BINARY_DIR}/${input_file_basename_we}-se.hh")
  string(MD5 md5 "paludis-seprocess-${output_header_file}")
  set(header_target_name paludis-seprocess-${md5}-${input_file_basename_we}-se.hh)
  set(${PSEP_HEADER_TARGET} ${header_target_name} PARENT_SCOPE)

  add_custom_command(OUTPUT
                       ${output_header_file}
                     COMMAND
                       "${PROJECT_SOURCE_DIR}/misc/make_se.bash" --header ${input_file} > ${output_header_file}
                     DEPENDS
                       "${PROJECT_SOURCE_DIR}/misc/make_se.bash"
                       ${input_file})
  add_custom_target(${header_target_name} DEPENDS ${output_header_file} COMMENT ${output_header_file})


  set(output_source_file "${CMAKE_CURRENT_BINARY_DIR}/${input_file_basename_we}-se.cc")
  string(MD5 md5 "paludis-seprocess-${output_source_file}")
  set(source_target_name paludis-seprocess-${md5}-${input_file_basename_we}-se.cc)
  set(${PSEP_SOURCE_TARGET} ${source_target_name} PARENT_SCOPE)

  add_custom_command(OUTPUT
                       ${output_source_file}
                     COMMAND
                       "${PROJECT_SOURCE_DIR}/misc/make_se.bash" --source ${input_file} > ${output_source_file}
                     DEPENDS
                       "${PROJECT_SOURCE_DIR}/misc/make_se.bash"
                       ${input_file})
  add_custom_target(${source_target_name} DEPENDS ${output_source_file} COMMENT ${output_source_file})
endfunction()

# TODO(compnerd) convert make_nn.bash into a cmake script
function(paludis_nnprocess input_file)
  set(options)
  set(single_value_args HEADER_TARGET HEADER_FILE SOURCE_TARGET SOURCE_FILE)
  set(multiple_value_args)

  cmake_parse_arguments(PNNP "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  if(NOT PNNP_HEADER_TARGET)
    message(SEND_ERROR "paludis_nnprocess(${input_file}) invoked without HEADER_TARGET option")
  endif()
  if(NOT PNNP_SOURCE_TARGET)
    message(SEND_ERROR "paludis_nnprocess(${input_file}) invoked without SOURCE_TARGET option")
  endif()

  get_filename_component(input_file_basename "${input_file}" NAME)
  get_filename_component(input_file_basename_we "${input_file}" NAME_WE)

  set(output_header_file "${CMAKE_CURRENT_BINARY_DIR}/${input_file_basename_we}-nn.hh")
  string(MD5 md5 "paludis-nnprocess-${output_header_file}")
  set(header_target_name paludis-nnprocess-${md5}-${input_file_basename_we}-nn.hh)

  add_custom_command(OUTPUT
                       ${output_header_file}
                     COMMAND
                       "${PROJECT_SOURCE_DIR}/misc/make_nn.bash" --header ${input_file} > ${output_header_file}
                     DEPENDS
                       "${PROJECT_SOURCE_DIR}/misc/make_nn.bash"
                       ${input_file})
  add_custom_target(${header_target_name} DEPENDS ${output_header_file} COMMENT ${output_header_file})


  set(output_source_file "${CMAKE_CURRENT_BINARY_DIR}/${input_file_basename_we}-nn.cc")
  string(MD5 md5 "paludis-nnprocess-${output_source_file}")
  set(source_target_name paludis-nnprocess-${md5}-${input_file_basename_we}-nn.cc)

  add_custom_command(OUTPUT
                       ${output_source_file}
                     COMMAND
                       "${PROJECT_SOURCE_DIR}/misc/make_nn.bash" --source ${input_file} > ${output_source_file}
                     DEPENDS
                       "${PROJECT_SOURCE_DIR}/misc/make_nn.bash"
                       ${input_file})
  add_custom_target(${source_target_name} DEPENDS ${output_source_file} COMMENT ${output_source_file})

  set(${PNNP_HEADER_TARGET} ${header_target_name} PARENT_SCOPE)
  if(PNNP_HEADER_FILE)
    set(${PNNP_HEADER_FILE} ${output_header_file} PARENT_SCOPE)
  endif()

  set(${PNNP_SOURCE_TARGET} ${source_target_name} PARENT_SCOPE)
  if(PNNP_SOURCE_FILE)
    set(${PNNP_SOURCE_FILE} ${output_source_file} PARENT_SCOPE)
  endif()
endfunction()

