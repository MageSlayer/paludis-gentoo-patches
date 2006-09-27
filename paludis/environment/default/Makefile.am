CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisdefaultenvironment_la_SOURCES = \
	default_config.cc      default_config.hh \
	default_environment.cc default_environment.hh

libpaludisdefaultenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
libpaludisdefaultenvironment_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

lib_LTLIBRARIES = libpaludisdefaultenvironment.la
paludis_environment_default_includedir = $(includedir)/paludis/environment/test
paludis_environment_default_include_HEADERS = default_config.hh default_environment.hh

