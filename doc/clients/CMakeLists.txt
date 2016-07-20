
paludis_generate_toplinks("..")
paludis_generate_header("..")
paludis_generate_footer("..")

if(cave IN_LIST PALUDIS_CLIENTS)
  set(ENABLE_CAVE_CLIENT FALSE)
else()
  set(ENABLE_CAVE_CLIENT TRUE)
endif()

function(paludis_generate_cave_command_html_doc subcommand)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html.part"
       "<h1>${subcommand}</h1>\n")
  if(NOT ENABLE_CAVE_CLIENT)
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html.part"
         "<p>Sorry, documentation was generated without support for the cave client.</p>\n")
  endif()
  paludis_cat("${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html"
                "${CMAKE_CURRENT_BINARY_DIR}/header.html.part"
                "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html.part"
                "${CMAKE_BINARY_DIR}/src/clients/cave/cave-${subcommand}.html-man-fragment"
                "${CMAKE_CURRENT_BINARY_DIR}/footer.html.part")
endfunction()

paludis_generate_page(index "${CMAKE_CURRENT_SOURCE_DIR}")

set(CAVE_SUBCOMMANDS
    config
    contents
    digest
    display-resolution
    dump-cave-formats-conf
    executables
    execute-resolution
    find-candidates
    fix-cache
    fix-linkage
    generate-metadata
    graph-jobs
    has-version
    help
    import
    info
    manage-search-index
    match
    mirror
    owner
    perform
    print-best-version
    print-categories
    print-checksum
    print-checksum-algorithms
    print-commands
    print-dependent-ids
    print-environment-metadata
    print-id-actions
    print-id-contents
    print-id-environment-variable
    print-id-executables
    print-id-masks
    print-id-metadata
    print-id-size
    print-ids
    print-owners
    print-packages
    print-repositories
    print-repository-formats
    print-repository-metadata
    print-resolution-required-confirmations
    print-set
    print-sets
    print-spec
    print-sync-protocols
    print-unmanaged-files
    print-unused-distfiles
    purge
    report
    resolve
    resume
    search
    show
    size
    sync
    sync-protocol-options
    uninstall
    update-world
    verify)

set(cave_documentation)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cave.html.head" "<h1>cave</h1>\n")
if(NOT ENABLE_CAVE_CLIENT)
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/cave.html.head"
       "<p>Sorry, documentation was generated without support for the cave client.</p>\n")
endif()
paludis_cat("${CMAKE_CURRENT_BINARY_DIR}/cave.html.part.in"
              "${CMAKE_CURRENT_BINARY_DIR}/cave.html.head"
              "${CMAKE_BINARY_DIR}/src/clients/cave/cave.html-man-fragment"
            DEPENDS
              cave-html-man-fragments)
paludis_sed(INPUT
              "${CMAKE_CURRENT_BINARY_DIR}/cave.html.part.in"
            OUTPUT
              "${CMAKE_CURRENT_BINARY_DIR}/cave.html.part"
            EXPRESSIONS
              "/<h2>SEE ALSO<.h2>/,/<.div>/s#<span class=\"strong\"><strong>\\(.*\\)</strong></span>(1)#<a href=\"\\1.html\">\\1</a>#"
              "/<h2>CORE COMMANDS<.h2>/,/<h2>ENVIRONMENT<.h2>/s,^\\([a-z-]\\+\\)$$,<a href=\"cave-\\1.html\">\\1</a>,")
paludis_cat("${CMAKE_CURRENT_BINARY_DIR}/cave.html"
              "${CMAKE_CURRENT_BINARY_DIR}/header.html.part"
              "${CMAKE_CURRENT_BINARY_DIR}/cave.html.part"
              "${CMAKE_CURRENT_BINARY_DIR}/footer.html.part")
list(APPEND cave_documentation "${CMAKE_CURRENT_BINARY_DIR}/cave.html")
foreach(subcommand ${CAVE_SUBCOMMANDS})
  paludis_generate_cave_command_html_doc(${subcommand})
  list(APPEND cave_documentation "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html")
endforeach()

add_custom_target(client-html-docs
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/index.html"
                    ${cave_documentation})
if(ENABLE_CAVE_CLIENT)
  add_dependencies(client-html-docs cave-html-man-fragments)
endif()

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/index.html"
          ${cave_documentation}
        DESTINATION
          "${CMAKE_INSTALL_FULL_HTMLDIR}/clients")

