
paludis_generate_toplinks("..")
paludis_generate_header("..")
paludis_generate_footer("..")

foreach(page
          contact
          features
          gettingstarted
          index
          pbins)
  paludis_generate_page(${page} "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

add_custom_target(overview-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/contact.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/features.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/gettingstarted.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/pbins.html")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/contact.html"
          "${CMAKE_CURRENT_BINARY_DIR}/features.html"
          "${CMAKE_CURRENT_BINARY_DIR}/gettingstarted.html"
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          "${CMAKE_CURRENT_BINARY_DIR}/pbins.html"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/overview")

