include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisunpackagedrepository.la

noinst_HEADERS = \
	exceptions.hh \
	unpackaged_id.hh \
	unpackaged_repository.hh \
	unpackaged_key.hh \
	unpackaged_stripper.hh \
	installed_repository.hh \
	installed_id.hh

libpaludisunpackagedrepository_la_SOURCES = \
	exceptions.cc \
	unpackaged_id.cc \
	unpackaged_repository.cc \
	unpackaged_key.cc \
	unpackaged_stripper.cc \
	installed_repository.cc \
	installed_id.cc \
	registration.cc \
	$(noinst_HEADERS)

unpackaged_repository_TEST_SOURCES = unpackaged_repository_TEST.cc

unpackaged_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

unpackaged_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

unpackaged_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

installed_repository_TEST_SOURCES = installed_repository_TEST.cc

installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

installed_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

EXTRA_DIST = \
	unpackaged_repository_TEST.cc \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST.cc \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh \
	installed_id.cc

check_SCRIPTS = \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh

TESTS = \
	unpackaged_repository_TEST \
	installed_repository_TEST

check_PROGRAMS = $(TESTS)

