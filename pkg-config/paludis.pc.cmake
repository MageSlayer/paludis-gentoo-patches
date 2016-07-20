prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: Paludis
Description: The Other Package Mangler
Version: @PALUDIS_PC_SLOT@
Libs: -L${libdir} @PALUDIS_PKG_CONFIG_LIBS@
Cflags: -I${includedir} @PALUDIS_PKG_CONFIG_CFLAGS@

