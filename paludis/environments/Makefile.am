SUBDIRS = . $(BUILD_ENVIRONMENTS)
DIST_SUBDIRS = $(ALL_ENVIRONMENTS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

lib_LTLIBRARIES = libpaludisenvironments.la

paludis_environments_includedir = $(includedir)/paludis/environments/

paludis_environments_include_HEADERS = \
	environment_maker.hh

libpaludisenvironments_la_SOURCES = \
	environment_maker.cc \
	$(paludis_environments_include_HEADERS)

libpaludisenvironments_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisenvironments_la_LIBADD = \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	@DYNAMIC_LD_LIBS@

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

