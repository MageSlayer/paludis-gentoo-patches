include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

noinst_LTLIBRARIES = libpaludisvirtualsrepository.la

noinst_HEADERS = \
	virtuals_repository.hh \
	installed_virtuals_repository.hh \
	package_id.hh

libpaludisvirtualsrepository_la_SOURCES = \
	virtuals_repository.cc \
	installed_virtuals_repository.cc \
	package_id.cc \
	registration.cc \
	$(noinst_HEADERS)

TESTS = virtuals_repository_TEST installed_virtuals_repository_TEST

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

EXTRA_DIST = $(virtuals_repository_TEST_SOURCES) $(installed_virtuals_repository_TEST_SOURCES)

virtuals_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

virtuals_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

virtuals_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

installed_virtuals_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

installed_virtuals_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

installed_virtuals_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

virtuals_repository_TEST_SOURCES = \
	virtuals_repository_TEST.cc

installed_virtuals_repository_TEST_SOURCES = \
	installed_virtuals_repository_TEST.cc

