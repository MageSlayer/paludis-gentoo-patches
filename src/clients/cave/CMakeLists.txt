
paludis_add_library(libcave
                    STATIC_LIBRARY
                      "${CMAKE_CURRENT_SOURCE_DIR}/colour_pretty_printer.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/command.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/command_line.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/command_command_line.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/command_factory.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_contents.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_config.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_digest.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_display_resolution.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_dump_cave_formats_conf.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_executables.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_execute_resolution.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_find_candidates.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_fix_cache.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_fix_linkage.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_generate_metadata.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_graph_jobs.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_has_version.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_help.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_import.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_info.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_manage_search_index.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_match.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_mirror.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_owner.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_perform.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_best_version.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_categories.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_checksum.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_checksum_algorithms.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_commands.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_dependent_ids.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_environment_metadata.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_actions.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_contents.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_environment_variable.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_executables.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_masks.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_metadata.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_id_size.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_ids.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_owners.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_packages.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_repositories.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_repository_formats.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_repository_metadata.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_resolution_required_confirmations.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_set.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_sets.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_spec.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_sync_protocols.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_unmanaged_files.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_print_unused_distfiles.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_purge.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_report.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_resolve.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_resolve_display_callback.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_resolve_dump.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_resume.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_search.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_search_cmdline.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_show.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_size.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_sync.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_sync_protocol_options.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_uninstall.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_update_world.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/cmd_verify.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/colours.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/exceptions.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/executables_common.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/format_package_id.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/format_plain_contents_entry.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/format_plain_metadata_key.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/format_string.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/format_user_config.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/script_command.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/search_extras_handle.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/select_format_for_spec.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/owner_common.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/parse_spec_with_nice_error.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/resolve_cmdline.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/resolve_common.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/resume_data.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/size_common.cc")
add_dependencies(libcave libpaludisargs_SE libpaludisresolver_SE libpaludisutil_SE)

paludis_add_library(libcavematchextras
                      "${CMAKE_CURRENT_SOURCE_DIR}/match_extras.cc")
target_link_libraries(libcavematchextras
                      PRIVATE
                        ${PCRECPP_LIBRARIES})

if(ENABLE_SEARCH_INDEX)
  paludis_add_library(libcavesearchextras
                        "${CMAKE_CURRENT_SOURCE_DIR}/search_extras.cc")
  target_compile_definitions(libcave
                             PRIVATE
                               -DENABLE_SEARCH_INDEX=1)
  target_link_libraries(libcavesearchextras
                        PRIVATE
                          libpaludisutil
                          ${SQLite3_LIBRARIES})
endif()

add_executable(cave
                 "${CMAKE_CURRENT_SOURCE_DIR}/cave.cc")
target_link_libraries(cave
                      PRIVATE
                        libcave
                        libpaludis
                        libpaludisargs
                        libpaludisutil
                        libpaludisresolver
                        liboutput
                        ${CMAKE_DL_LIBS})

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

if(NOT USE_PREBUILT_DOCUMENTATION)
  add_executable(man-cave
                   "${CMAKE_CURRENT_SOURCE_DIR}/man_cave.cc")
  target_link_libraries(man-cave
                        PRIVATE
                          libcave
                          libpaludis
                          libpaludisargs
                          libpaludisutil
                          libpaludisresolver
                          ${CMAKE_DL_LIBS})

  add_custom_command(OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.txt"
                     COMMAND
                       man-cave > "${CMAKE_CURRENT_BINARY_DIR}/cave.txt"
                     DEPENDS
                       man-cave)
  add_custom_command(OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.xml"
                     COMMAND
                       asciidoc --doctype=manpage --backend=docbook --out-file "${CMAKE_CURRENT_BINARY_DIR}/cave.xml" --conf "${PROJECT_SOURCE_DIR}/misc/asciidoc.conf" "${CMAKE_CURRENT_BINARY_DIR}/cave.txt"
                     DEPENDS
                       "${PROJECT_SOURCE_DIR}/misc/asciidoc.conf"
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.txt")
  add_custom_command(OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.1"
                     COMMAND
                       xmlto man "${CMAKE_CURRENT_BINARY_DIR}/cave.xml"
                     DEPENDS
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.xml")
  add_custom_command(OUTPUT
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment"
                     COMMAND
                       xmlto -o "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment-dir" html "${CMAKE_CURRENT_BINARY_DIR}/cave.xml"
                     COMMAND
                       "${SED_EXECUTABLE}" -e "'1s,^.*\\(<div class=\"refnameddiv\">\\),\\1,'" -e "'$$s,</body>.*,,'" -e "'$$s,</dev><div class=\"navfooter\"><hr></div>,,'" -e "'s,<a name=\"[^\"]*\"></a>,,g'" < "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment-dir/index.html" > "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment"
                     BYPRODUCTS
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment-dir/cave.proc"
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment-dir/index.html"
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment-dir"
                     DEPENDS
                       "${CMAKE_CURRENT_BINARY_DIR}/cave.xml")

  foreach(subcommand ${CAVE_SUBCOMMANDS})
    add_custom_command(OUTPUT
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.txt"
                       COMMAND
                         man-cave ${subcommand} > "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.txt"
                       DEPENDS
                         man-cave)
    add_custom_command(OUTPUT
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml"
                       COMMAND
                         asciidoc --doctype=manpage --backend=docbook --out-file "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml" --conf "${PROJECT_SOURCE_DIR}/misc/asciidoc.conf" "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.txt"
                       DEPENDS
                         "${PROJECT_SOURCE_DIR}/misc/asciidoc.conf"
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.txt")
    add_custom_command(OUTPUT
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.1"
                       COMMAND
                         xmlto man "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml"
                       DEPENDS
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml")
    add_custom_command(OUTPUT
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment"
                       COMMAND
                         xmlto -o "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment-dir" html "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml"
                       COMMAND
                         "${SED_EXECUTABLE}" -e "'1s,^.*\\(<div class=\"refnameddiv\">\\),\\1,'" -e "'$$s,</body>.*,,'" -e "'$$s,</dev><div class=\"navfooter\"><hr></div>,,'" -e "'s,<a name=\"[^\"]*\"></a>,,g'" < "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment-dir/index.html" > "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment"
                       BYPRODUCTS
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment-dir/cave-${subcommand}.proc"
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment-dir/index.html"
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment-dir"
                       DEPENDS
                         "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.xml")
  endforeach()

  set(html-fragments "${CMAKE_CURRENT_BINARY_DIR}/cave.html-man-fragment")
  set(manpages "${CMAKE_CURRENT_BINARY_DIR}/cave.1")
  foreach(subcommand ${CAVE_SUBCOMMANDS})
    list(APPEND html-fragments "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.html-man-fragment")
    list(APPEND manpages "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.1")
  endforeach()
  add_custom_target(cave-manpages
                    ALL
                    DEPENDS
                      ${manpages})
  add_custom_target(cave-html-man-fragments
                    DEPENDS
                      ${html-fragments})
endif()

paludis_add_test(continue_on_failure BASH)

install(TARGETS
          cave
        DESTINATION
          "${CMAKE_INSTALL_FULL_BINDIR}")
install(TARGETS
          libcavematchextras
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}")
if(ENABLE_SEARCH_INDEX)
  install(TARGETS
            libcavesearchextras
          DESTINATION
            "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()
install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/cave.1"
        DESTINATION
          "${CMAKE_INSTALL_FULL_MANDIR}/man1")
foreach(subcommand ${CAVE_SUBCOMMANDS})
  install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/cave-${subcommand}.1"
          DESTINATION
            "${CMAKE_INSTALL_FULL_MANDIR}/man1")
endforeach()
install(PROGRAMS
          "${CMAKE_CURRENT_SOURCE_DIR}/moo"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/cave/commands")

