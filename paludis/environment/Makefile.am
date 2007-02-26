SUBDIRS = default no_config test

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

lib_LTLIBRARIES = libpaludisenvironment.la

paludis_environment_includedir = $(includedir)/paludis/environment/

paludis_environment_include_HEADERS = \
	environment_maker.hh

libpaludisenvironment_la_SOURCES = \
	environment_maker.cc \
	$(paludis_environment_include_HEADERS)

libpaludisenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisenvironment_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	@DYNAMIC_LD_LIBS@

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"


