include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisunpackagedrepository.la

noinst_HEADERS = \
	dep_parser.hh \
	dep_printer.hh \
	exceptions.hh \
	unpackaged_id.hh \
	unpackaged_repository.hh \
	unpackaged_key.hh \
	unpackaged_stripper.hh \
	installed_repository.hh \
	installed_id.hh

libpaludisunpackagedrepository_la_SOURCES = \
	dep_parser.cc \
	dep_printer.cc \
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
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

unpackaged_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

installed_repository_TEST_SOURCES = installed_repository_TEST.cc

installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

dep_parser_TEST_SOURCES = dep_parser_TEST.cc

dep_parser_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

dep_parser_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

EXTRA_DIST = \
	unpackaged_repository_TEST.cc \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST.cc \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh \
	installed_id.cc \
	dep_parser_TEST.cc

check_SCRIPTS = \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh

TESTS = \
	unpackaged_repository_TEST \
	installed_repository_TEST \
	dep_parser_TEST

check_PROGRAMS = $(TESTS)

