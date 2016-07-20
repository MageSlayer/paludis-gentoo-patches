
foreach(scheme bzr;bzr+aftp;bzr+file;bzr+ftp;bzr+http;bzr+https;bzr+lp;bzr+sftp;bzr+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dobzr.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme cvs+ext;cvs+pserver;cvs+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/docvs.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme darcs+file;darcs+http;darcs+https;darcs+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dodarcs.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme git;git+file;git+http;git+https;git+rsync;git+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dogit.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme hg+file;hg+http;hg+https;hg+ssh;hg+static-http)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dohg.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme file;rsync;rsync+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dorsync.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme svn;svn+file;svn+http;svn+https;svn+ssh)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dosvn.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()
foreach(scheme tar+file;tar+ftp;tar+http;tar+https)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dotar.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/do${scheme}"
                 @ONLY)
endforeach()

add_custom_command(OUTPUT
                     "${CMAKE_CURRENT_BINARY_DIR}/.keep"
                   COMMAND
                     "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_BINARY_DIR}/.keep")
# FIXME(compnerd) auto-generate the target
add_custom_target(syncers-keep
                  ALL
                  DEPENDS
                    "${CMAKE_CURRENT_BINARY_DIR}/.keep")

install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+aftp"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+ftp"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+https"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+lp"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+sftp"
          "${CMAKE_CURRENT_BINARY_DIR}/dobzr+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/docvs+ext"
          "${CMAKE_CURRENT_BINARY_DIR}/docvs+pserver"
          "${CMAKE_CURRENT_BINARY_DIR}/docvs+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dodarcs+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dodarcs+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dodarcs+https"
          "${CMAKE_CURRENT_BINARY_DIR}/dodarcs+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dofile"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit+https"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit+rsync"
          "${CMAKE_CURRENT_BINARY_DIR}/dogit+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dohg+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dohg+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dohg+https"
          "${CMAKE_CURRENT_BINARY_DIR}/dohg+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dohg+static-http"
          "${CMAKE_CURRENT_BINARY_DIR}/dorsync"
          "${CMAKE_CURRENT_BINARY_DIR}/dorsync+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dosvn"
          "${CMAKE_CURRENT_BINARY_DIR}/dosvn+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dosvn+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dosvn+https"
          "${CMAKE_CURRENT_BINARY_DIR}/dosvn+ssh"
          "${CMAKE_CURRENT_BINARY_DIR}/dotar+file"
          "${CMAKE_CURRENT_BINARY_DIR}/dotar+ftp"
          "${CMAKE_CURRENT_BINARY_DIR}/dotar+http"
          "${CMAKE_CURRENT_BINARY_DIR}/dotar+https"
          "${CMAKE_CURRENT_SOURCE_DIR}/dodummy"
        DESTINATION
          "${CMAKE_INSTALL_FULL_LIBEXECDIR}/paludis/syncers")
install(FILES
          "${CMAKE_CURRENT_BINARY_DIR}/.keep"
        DESTINATION
          "${CMAKE_INSTALL_FULL_DATADIR}/paludis/syncers")

