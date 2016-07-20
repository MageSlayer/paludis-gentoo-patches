
include(CMakeParseArguments)

function(paludis_cat destination)
  set(options)
  set(single_value_args)
  set(multiple_value_args DEPENDS)

  cmake_parse_arguments(PC "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  add_custom_command(OUTPUT
                       "${destination}"
                     COMMAND
                       "${CAT_EXECUTABLE}" ${PC_UNPARSED_ARGUMENTS} > ${destination}
                     DEPENDS
                       ${PC_UNPARSED_ARGUMENTS}
                       ${PC_DEPENDS})
endfunction()

function(paludis_sed)
  set(options)
  set(single_value_args INPUT OUTPUT)
  set(multiple_value_args DEPENDS EXPRESSIONS)

  cmake_parse_arguments(PS "${options}" "${single_value_args}" "${multiple_value_args}" ${ARGN})

  set(sed_command ${SED_EXECUTABLE})
  foreach(expression ${PS_EXPRESSIONS})
    list(APPEND sed_command -e;"${expression}")
  endforeach()

  add_custom_command(OUTPUT
                       "${PS_OUTPUT}"
                     COMMAND
                       ${sed_command} < "${PS_INPUT}" > "${PS_OUTPUT}"
                     DEPENDS
                       ${PS_INPUT}
                       ${PS_DEPENDS})
endfunction()

function(paludis_generate_toplinks relative)
  paludis_sed(INPUT
                "${CMAKE_CURRENT_SOURCE_DIR}/${relative}/toplinks.html.part.in"
              OUTPUT
                "${CMAKE_CURRENT_BINARY_DIR}/toplinks.html.part"
              EXPRESSIONS
                "s,###TOPURI###,${relative}/,g")
endfunction()
function(paludis_generate_header relative)
  paludis_sed(INPUT
                "${CMAKE_CURRENT_SOURCE_DIR}/${relative}/header.html.part.in"
              OUTPUT
                "${CMAKE_CURRENT_BINARY_DIR}/header.html.part"
              DEPENDS
                "${CMAKE_CURRENT_BINARY_DIR}/toplinks.html.part"
              EXPRESSIONS
                "s,###TOPURI###,${relative}/,g"
                "/###TOPLINKS###/r ${CMAKE_CURRENT_SOURCE_DIR}/toplinks.html.part"
                "s,###TOPLINKS###,,g")
endfunction()
function(paludis_generate_footer relative)
  paludis_sed(INPUT
                "${CMAKE_CURRENT_SOURCE_DIR}/${relative}/footer.html.part.in"
              OUTPUT
                "${CMAKE_CURRENT_BINARY_DIR}/footer.html.part"
              DEPENDS
                "${CMAKE_CURRENT_BINARY_DIR}/toplinks.html.part"
              EXPRESSIONS
                "s,###TOPURI###,${relative}/,g")
endfunction()
function(paludis_generate_page page location)
  paludis_cat("${CMAKE_CURRENT_BINARY_DIR}/${page}.html"
                "${CMAKE_CURRENT_BINARY_DIR}/header.html.part"
                "${location}/${page}.html.part"
                "${CMAKE_CURRENT_BINARY_DIR}/footer.html.part")
endfunction()

paludis_generate_toplinks(".")
paludis_generate_header(".")
paludis_generate_footer(".")

paludis_sed(INPUT
              "${CMAKE_CURRENT_SOURCE_DIR}/index.html.part.in"
            OUTPUT
              "${CMAKE_CURRENT_BINARY_DIR}/index.html.part"
            EXPRESSIONS
              "s,###PALUDIS_VERSION###,${PROJECT_VERSION},g")
paludis_generate_page(index "${CMAKE_CURRENT_BINARY_DIR}")
paludis_sed(INPUT
              "${PROJECT_SOURCE_DIR}/NEWS"
            OUTPUT
              "${CMAKE_CURRENT_BINARY_DIR}/news.subst"
            EXPRESSIONS
              "s,&,\&amp\\\\\\;,g"
              "s,<,\&lt\\\\\\;,g"
              "s,>,\&gt\\\\\\;,g")
paludis_sed(INPUT
              "${CMAKE_CURRENT_SOURCE_DIR}/news.html.part.in"
            OUTPUT
              "${CMAKE_CURRENT_BINARY_DIR}/news.html.part"
            DEPENDS
              "${CMAKE_CURRENT_BINARY_DIR}/news.subst"
            EXPRESSIONS
              "/###NEWS###/r ${CMAKE_CURRENT_BINARY_DIR}/news.subst"
              "s,###NEWS###,,")
paludis_generate_page(news "${CMAKE_CURRENT_BINARY_DIR}")
execute_process(COMMAND
                  "${GIT_EXECUTABLE}" --git-dir="${PROJECT_SOURCE_DIR}/.git" log
                COMMAND
                  "${SED_EXECUTABLE}" -e "'s,&,\&amp;,g'"
                                      -e "'s,<,\&lt;,g'"
                                      -e "'s,>,\&gt;,g'"
                                      -e "'s,\\([[:blank:][:punct:]]\\)\\?\\([0-9a-f]\\{40\\}\\)\\([[:blank:][:punct:]]\)\\?,\\1<a href=\"http://git.exherbo.org/paludis/paludis.git/commit/?id=\\2\">\\2</a>,g'"
                                      -e "'s,ticket:\\([0-9]\\+\\),<a href=\"http://paludis.exherbo.org/trac/ticket/\\1\">ticket:\\1</a>,g'"
                OUTPUT_FILE
                  "${CMAKE_CURRENT_BINARY_DIR}/changelog.subst"
                ERROR_QUIET
                OUTPUT_STRIP_TRAILING_WHITESPACE)
paludis_sed(INPUT
              "${CMAKE_CURRENT_SOURCE_DIR}/changelog.html.part.in"
            OUTPUT
              "${CMAKE_CURRENT_BINARY_DIR}/changelog.html.part"
            DEPENDS
              "${CMAKE_CURRENT_BINARY_DIR}/changelog.subst"
            EXPRESSIONS
              "/###CHANGELOG###/r ${CMAKE_CURRENT_BINARY_DIR}/changelog.subst"
              "s,###CHANGELOG###,,")
paludis_generate_page(changelog "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/news.html"
                    "${CMAKE_CURRENT_BINARY_DIR}/changelog.html")

add_subdirectory(api)
add_subdirectory(clients)
add_subdirectory(configuration)
add_subdirectory(faq)
add_subdirectory(overview)

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          "${CMAKE_CURRENT_BINARY_DIR}/news.html"
          "${CMAKE_CURRENT_BINARY_DIR}/changelog.html"
          "${CMAKE_CURRENT_SOURCE_DIR}/paludis.css"
          "${CMAKE_CURRENT_SOURCE_DIR}/paludis_270.png"
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}")

