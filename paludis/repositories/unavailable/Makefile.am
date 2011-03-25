include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisunavailablerepository.la

noinst_HEADERS = \
	unavailable_repository.hh unavailable_repository-fwd.hh \
	unavailable_package_id.hh \
	unavailable_repository_id.hh \
	unavailable_repository_dependencies_key.hh \
	unavailable_mask.hh \
	unavailable_repository_store.hh \
	unavailable_repository_file.hh unavailable_repository_file-fwd.hh

libpaludisunavailablerepository_la_SOURCES = \
	unavailable_repository.cc \
	unavailable_package_id.cc \
	unavailable_repository_id.cc \
	unavailable_repository_dependencies_key.cc \
	unavailable_mask.cc \
	unavailable_repository_store.cc \
	unavailable_repository_file.cc \
	registration.cc \
	$(noinst_HEADERS)

unavailable_repository_TEST_SOURCES = unavailable_repository_TEST.cc

unavailable_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

unavailable_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

unavailable_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

EXTRA_DIST = \
	unavailable_repository_TEST.cc \
	unavailable_repository_TEST_setup.sh \
	unavailable_repository_TEST_cleanup.sh

check_SCRIPTS = \
	unavailable_repository_TEST_setup.sh \
	unavailable_repository_TEST_cleanup.sh

TESTS = \
	unavailable_repository_TEST

check_PROGRAMS = $(TESTS)

