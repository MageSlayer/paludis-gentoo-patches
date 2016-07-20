
if(PALUDIS_COLOUR_PINK)
  add_definitions(-DPALUDIS_COLOUR_PINK=0)
else()
  add_definitions(-DPALUDIS_COLOUR_PINK=1)
endif()

add_subdirectory(output)
add_subdirectory(clients)

