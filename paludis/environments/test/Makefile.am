CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda ihateautomake.cc ihateautomake.o
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludistestenvironment_la_SOURCES = \
	test_environment.cc test_environment.hh \
	registration.cc

libpaludistestenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ! MONOLITHIC

libpaludistestenvironment_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

endif

if MONOLITHIC

noinst_LTLIBRARIES = libpaludistestenvironment.la

else

lib_LTLIBRARIES = libpaludistestenvironment.la

endif

paludis_environment_test_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/test
paludis_environment_test_include_HEADERS = test_environment.hh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done


