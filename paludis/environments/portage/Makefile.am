include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisportageenvironment_la_SOURCES = \
	registration.cc \
	portage_environment.cc portage_environment.hh

libexecenvdir = $(libexecdir)/paludis/environments/portage/
libexecenv_SCRIPTS = bashrc

noinst_LTLIBRARIES = libpaludisportageenvironment.la

noinst_HEADERS = \
	portage_environment.hh

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	$(libexecenv_SCRIPTS) \
	portage_environment_TEST_setup.sh \
	portage_environment_TEST.cc \
	portage_environment_TEST_cleanup.sh

TESTS = portage_environment_TEST

portage_environment_TEST_SOURCES = portage_environment_TEST.cc

portage_environment_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

portage_environment_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

portage_environment_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

check_PROGRAMS = $(TESTS)
check_SCRIPTS = portage_environment_TEST_setup.sh portage_environment_TEST_cleanup.sh

