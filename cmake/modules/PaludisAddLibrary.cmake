
include(CMakeParseArguments)
include(PaludisGeneratorUtils)

function(paludis_add_library library_name)
  set(options SHARED_LIBRARY STATIC_LIBRARY OBJECT_LIBRARY UNVERSIONED)
  set(single_value_args SE_HEADERS)
  set(multiple_value_args NN_SOURCES SE_SOURCES INCORPORATE_OBJECT_LIBRARIES)

  cmake_parse_arguments(PAL "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  if(PAL_STATIC_LIBRARY AND PAL_OBJECT_LIBRARY)
    message(SEND_ERROR "paludis_add_library(${library_name}) called with STATIC_LIBRARY and OBJECT_LIBRARY")
  endif()

  if(PAL_STATIC_LIBRARY)
    set(libkind STATIC)
  elseif(PAL_OBJECT_LIBRARY)
    set(libkind OBJECT)
  elseif(PAL_SHARED_LIBRARY)
    set(libkind SHARED)
  else()
    set(libkind)
  endif()

  set(nnsources)
  set(nndependencies)
  foreach(nn_source ${PAL_NN_SOURCES})
    get_filename_component(nnname ${nn_source} NAME_WE)
    paludis_nnprocess(${nn_source}
                      HEADER_TARGET
                        ${nnname}_HEADER_TARGET
                      SOURCE_TARGET
                        ${nnname}_SOURCE_TARGET
                      SOURCE_FILE
                        ${nnname}_SOURCE_FILE)
    list(APPEND nnsources ${${nnname}_SOURCE_FILE})
    list(APPEND nndependencies ${${nnname}_HEADER_TARGET};${${nnname}_SOURCE_TARGET})
  endforeach()
  add_custom_target(${library_name}_NN
                    DEPENDS
                      ${nndependencies})

  set(seheaders)
  set(sedependencies)
  foreach(se_source ${PAL_SE_SOURCES})
    paludis_seprocess(${se_source}
                      HEADER_TARGET
                        ${se_source}_HEADER_TARGET
                      SOURCE_TARGET
                        ${se_source}_SOURCE_TARGET
                      HEADER_FILE
                        ${se_source}_HEADER_FILE)
    list(APPEND seheaders ${${se_source}_HEADER_FILE})
    list(APPEND sedependencies ${${se_source}_HEADER_TARGET};${${se_source}_SOURCE_TARGET})
  endforeach()
  add_custom_target(${library_name}_SE
                    DEPENDS
                      ${sedependencies})
  if(PAL_SE_HEADERS)
    set(${PAL_SE_HEADERS} ${seheaders} PARENT_SCOPE)
  endif()

  set(object_libraries_expressions)
  foreach(library ${PAL_INCORPORATE_OBJECT_LIBRARIES})
    list(APPEND object_libraries_expressions $<TARGET_OBJECTS:${library}>)
  endforeach()

  add_library(${library_name}
              ${libkind}
                ${PAL_UNPARSED_ARGUMENTS}
                ${object_libraries_expressions}
                ${nnsources})
  if(nndependencies)
    add_dependencies(${library_name} ${nndependencies})
  endif()
  if(sedependencies)
    add_dependencies(${library_name} ${sedependencies})
  endif()

  if(NOT PAL_OBJECT_LIBRARY)
    get_target_property(libkind ${library_name} TYPE)
    string(REGEX REPLACE "^lib" "" output_name ${library_name})
    set(output_name ${output_name}_${PALUDIS_PKG_CONFIG_SLOT})
    set_target_properties(${library_name} PROPERTIES OUTPUT_NAME ${output_name})
  endif()

  if("${libkind}" STREQUAL "SHARED_LIBRARY" AND NOT PAL_UNVERSIONED)
    math(EXPR version_major "${PROJECT_VERSION_MAJOR} * 100 + ${PROJECT_VERSION_MINOR}")
    set_target_properties(${library_name}
                          PROPERTIES
                            VERSION
                              "${version_major}.${PROJECT_VERSION_PATCH}.0"
                            SOVERSION
                              "${version_major}")
  endif()
endfunction()

