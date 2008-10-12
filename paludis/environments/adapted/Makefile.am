CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT ihateautomake.cc ihateautomake.o *.epicfail
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisadaptedenvironment_@PALUDIS_PC_SLOT@_la_SOURCES = \
	adapted_environment.cc adapted_environment.hh \
	registration.cc

libpaludisadaptedenvironment_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisadaptedenvironment_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludisadaptedenvironment_@PALUDIS_PC_SLOT@.la

paludis_environment_adapted_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/adapted
paludis_environment_adapted_include_HEADERS = \
	adapted_environment.hh

EXTRA_DIST = \
	adapted_environment_TEST.cc

TESTS = adapted_environment_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

adapted_environment_TEST_SOURCES = adapted_environment_TEST.cc

adapted_environment_TEST_LDADD = \
	libpaludisadaptedenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

BUILT_SOURCES =

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

