
paludis_generate_toplinks("..")
paludis_generate_header("..")
paludis_generate_footer("..")

foreach(page
          different
          general
          index
          howdoi
          misfunctionality
          operation
          repositories
          stricter
          upgrades)
  paludis_generate_page(${page} "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

add_custom_target(faq-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/different.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/general.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/howdoi.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/misfunctionality.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/operation.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/repositories.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/stricter.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/upgrades.html")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/different.html"
          "${CMAKE_CURRENT_BINARY_DIR}/general.html"
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          "${CMAKE_CURRENT_BINARY_DIR}/howdoi.html"
          "${CMAKE_CURRENT_BINARY_DIR}/misfunctionality.html"
          "${CMAKE_CURRENT_BINARY_DIR}/operation.html"
          "${CMAKE_CURRENT_BINARY_DIR}/repositories.html"
          "${CMAKE_CURRENT_BINARY_DIR}/stricter.html"
          "${CMAKE_CURRENT_BINARY_DIR}/upgrades.html"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/faq")

