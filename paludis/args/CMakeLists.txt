
paludis_add_library(libpaludisargs
                      "${CMAKE_CURRENT_SOURCE_DIR}/args.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_error.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_group.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_handler.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_option.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_section.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/bad_argument.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_visitor.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_dumper.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/escape.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/log_level_arg.cc"
                      "${CMAKE_CURRENT_SOURCE_DIR}/man.cc"
                    SE_SOURCES
                      "${CMAKE_CURRENT_SOURCE_DIR}/args_handler.se")
target_link_libraries(libpaludisargs
                      PRIVATE
                        libpaludisutil)

paludis_add_test(args_TEST GTEST
                 LINK_LIBRARIES
                   libpaludisargs)

install(TARGETS
          libpaludisargs
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBDIR}")
install(FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/args_dumper.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_error.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_group.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_handler.hh"
          "${CMAKE_CURRENT_BINARY_DIR}/args_handler-se.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_option.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_section.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/args_visitor.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/bad_argument.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/do_help.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/escape.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/log_level_arg.hh"
          "${CMAKE_CURRENT_SOURCE_DIR}/man.hh"
        DESTINATION
          "${CMAKE_INSTALL_FULL_INCLUDEDIR}/paludis-${PALUDIS_PKG_CONFIG_SLOT}/paludis/args")

