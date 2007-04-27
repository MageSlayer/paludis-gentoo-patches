CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda ihateautomake.cc ihateautomake.o
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisadaptedenvironment_la_SOURCES = \
	adapted_environment.cc adapted_environment.hh \
	registration.cc

libpaludisadaptedenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisadaptedenvironment_la_LIBADD = \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

lib_LTLIBRARIES = libpaludisadaptedenvironment.la

paludis_environment_adapted_includedir = $(includedir)/paludis/environments/adapted
paludis_environment_adapted_include_HEADERS = \
	adapted_environment.hh

EXTRA_DIST = \
	adapted_environment_TEST.cc

TESTS = adapted_environment_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

adapted_environment_TEST_SOURCES = adapted_environment_TEST.cc

adapted_environment_TEST_LDADD = \
	libpaludisadaptedenvironment.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

BUILT_SOURCES =

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done


