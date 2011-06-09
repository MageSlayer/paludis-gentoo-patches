include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisunwrittenrepository.la

noinst_HEADERS = \
	unwritten_repository.hh \
	unwritten_id.hh \
	unwritten_mask.hh \
	unwritten_repository_store.hh \
	unwritten_repository_file.hh

libpaludisunwrittenrepository_la_SOURCES = \
	unwritten_repository.cc \
	unwritten_id.cc \
	unwritten_mask.cc \
	unwritten_repository_store.cc \
	unwritten_repository_file.cc \
	registration.cc \
	$(noinst_HEADERS)

unwritten_repository_TEST_SOURCES = unwritten_repository_TEST.cc

unwritten_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

unwritten_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

unwritten_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

EXTRA_DIST = \
	unwritten_repository_TEST.cc \
	unwritten_repository_TEST_setup.sh \
	unwritten_repository_TEST_cleanup.sh

check_SCRIPTS = \
	unwritten_repository_TEST_setup.sh \
	unwritten_repository_TEST_cleanup.sh

TESTS = \
	unwritten_repository_TEST

check_PROGRAMS = $(TESTS)

