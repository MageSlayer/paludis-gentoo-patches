
add_subdirectory(repositories)

file(GLOB syncers
     LIST_DIRECTORIES FALSE
     "${CMAKE_SOURCE_DIR}/paludis/syncers/do*"
     "${CMAKE_BINARY_DIR}/paludis/syncers/do*")
list(SORT syncers)
foreach(syncer ${syncers})
  get_filename_component(syncer ${syncer} NAME)
  if(NOT ${syncer} MATCHES ".in")
    string(REPLACE "do" "" syncer ${syncer})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/synclist" "<strong>${syncer}</strong>\n")
  endif()
endforeach()

file(GLOB fetchers
     LIST_DIRECTORIES FALSE
     "${CMAKE_SOURCE_DIR}/paludis/fetchers/do*"
     "${CMAKE_BINARY_DIR}/paludis/fetchers/do*")
list(SORT fetchers)
foreach(fetcher ${fetchers})
  get_filename_component(fetcher ${fetcher} NAME)
  if(NOT ${fetcher} MATCHES ".in")
    string(REPLACE "do" "" fetcher ${fetcher})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/fetchlist" "<li>${fetcher}</li>\n")
  endif()
endforeach()

paludis_generate_toplinks("..")
paludis_generate_header("..")
paludis_generate_footer("..")

file(READ ${CMAKE_CURRENT_BINARY_DIR}/fetchlist FETCHERS)
configure_file(fetchers.html.part.in fetchers.html.part @ONLY)
paludis_generate_page(fetchers "${CMAKE_CURRENT_BINARY_DIR}")

file(READ ${CMAKE_CURRENT_BINARY_DIR}/synclist SYNCERS)
configure_file(syncers.html.part.in syncers.html.part @ONLY)
paludis_generate_page(syncers "${CMAKE_CURRENT_BINARY_DIR}")

file(READ paludisenvironmentonly.html.part PALUDISENVIRONMENTONLY)
foreach(page
          bashrc
          general
          keywords
          licenses
          mirrors
          output
          packagemask
          sets
          specpath
          suggestions
          use)
  configure_file(${page}.html.part.in ${page}.html.part @ONLY)
  paludis_generate_page(${page} "${CMAKE_CURRENT_BINARY_DIR}")
endforeach()

foreach(page
          configfiles
          envvars
          hooks
          index
          specs)
  paludis_generate_page(${page} "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()

add_custom_target(configuration-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/bashrc.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/configfiles.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/envvars.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/fetchers.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/general.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/hooks.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/keywords.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/licenses.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/mirrors.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/output.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/packagemask.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/sets.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/specpath.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/specs.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/suggestions.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/syncers.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/use.html")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/bashrc.html"
          "${CMAKE_CURRENT_BINARY_DIR}/configfiles.html"
          "${CMAKE_CURRENT_BINARY_DIR}/envvars.html"
          "${CMAKE_CURRENT_BINARY_DIR}/fetchers.html"
          "${CMAKE_CURRENT_BINARY_DIR}/general.html"
          "${CMAKE_CURRENT_BINARY_DIR}/hooks.html"
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          "${CMAKE_CURRENT_BINARY_DIR}/keywords.html"
          "${CMAKE_CURRENT_BINARY_DIR}/licenses.html"
          "${CMAKE_CURRENT_BINARY_DIR}/mirrors.html"
          "${CMAKE_CURRENT_BINARY_DIR}/output.html"
          "${CMAKE_CURRENT_BINARY_DIR}/packagemask.html"
          "${CMAKE_CURRENT_BINARY_DIR}/sets.html"
          "${CMAKE_CURRENT_BINARY_DIR}/specpath.html"
          "${CMAKE_CURRENT_BINARY_DIR}/specs.html"
          "${CMAKE_CURRENT_BINARY_DIR}/suggestions.html"
          "${CMAKE_CURRENT_BINARY_DIR}/syncers.html"
          "${CMAKE_CURRENT_BINARY_DIR}/use.html"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/configuration")

