
foreach(repository ${PALUDIS_ALL_REPOSITORIES})
  string(TOUPPER ${repository} repository_uppercase)
  if(ENABLE_${repository_uppercase}_REPOSITORY)
    add_subdirectory(${repository})
  endif()
endforeach()

