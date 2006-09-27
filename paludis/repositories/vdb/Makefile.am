CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = vdb_repository-sr.hh vdb_repository-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisvdbrepository.la
libpaludisvdbrepository_la_SOURCES = vdb_repository.cc vdb_repository.hh
libpaludisvdbrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
paludis_repositories_vdb_includedir = $(includedir)/paludis/repositories/vdb/
paludis_repositories_vdb_include_HEADERS = \
	vdb_repository.hh \
	vdb_repository-sr.hh \
	vdb_repository-sr.cc

libpaludisvdbrepository_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

TESTS = vdb_repository_TEST

vdb_repository_TEST_SOURCES = vdb_repository_TEST.cc

vdb_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	libpaludisvdbrepository.la

vdb_repository_TEST_CXXFLAGS = -I$(top_srcdir)

EXTRA_DIST = \
	vdb_repository_TEST.cc \
	vdb_repository_TEST_setup.sh \
	vdb_repository_TEST_cleanup.sh \
	vdb_repository-sr.hh \
	vdb_repository-sr.cc \
	vdb_repository.sr

BUILT_SOURCES = \
	vdb_repository-sr.hh \
	vdb_repository-sr.cc

check_PROGRAMS = $(TESTS)
check_SCRIPTS = vdb_repository_TEST_setup.sh vdb_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

vdb_repository-sr.hh : vdb_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/vdb_repository.sr > $@

vdb_repository-sr.cc : vdb_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/vdb_repository.sr > $@


