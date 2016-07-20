
add_subdirectory(cplusplus)
add_subdirectory(python)
add_subdirectory(ruby)

paludis_generate_toplinks("..")
paludis_generate_header("..")
paludis_generate_footer("..")

paludis_generate_page(index "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(api-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/api")

