
paludis_generate_toplinks("../..")
paludis_generate_header("../..")
paludis_generate_footer("../..")

foreach(page
          accounts
          e
          exndbam
          index
          installed_accounts
          installed_unpackaged
          repository
          unavailable
          unwritten
          vdb)
  paludis_generate_page(${page} "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

add_custom_target(configuration-repositories-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/accounts.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/e.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/exndbam.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/installed_accounts.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/installed_unpackaged.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/repository.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/unavailable.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/unwritten.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/vdb.html")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/accounts.html"
          "${CMAKE_CURRENT_BINARY_DIR}/e.html"
          "${CMAKE_CURRENT_BINARY_DIR}/exndbam.html"
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          "${CMAKE_CURRENT_BINARY_DIR}/installed_accounts.html"
          "${CMAKE_CURRENT_BINARY_DIR}/installed_unpackaged.html"
          "${CMAKE_CURRENT_BINARY_DIR}/repository.html"
          "${CMAKE_CURRENT_BINARY_DIR}/unavailable.html"
          "${CMAKE_CURRENT_BINARY_DIR}/unwritten.html"
          "${CMAKE_CURRENT_BINARY_DIR}/vdb.html"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/configuration/repositories")

