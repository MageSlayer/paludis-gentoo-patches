CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT ihateautomake.cc ihateautomake.o
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludistestenvironment_@PALUDIS_PC_SLOT@_la_SOURCES = \
	test_environment.cc test_environment.hh \
	registration.cc

libpaludistestenvironment_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ! MONOLITHIC

libpaludistestenvironment_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

endif

if MONOLITHIC

noinst_LTLIBRARIES = libpaludistestenvironment_@PALUDIS_PC_SLOT@.la

else

lib_LTLIBRARIES = libpaludistestenvironment_@PALUDIS_PC_SLOT@.la

endif

paludis_environment_test_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/test
paludis_environment_test_include_HEADERS = test_environment.hh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
