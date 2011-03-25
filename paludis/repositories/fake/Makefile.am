include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

TESTS = fake_repository_TEST fake_installed_repository_TEST dep_parser_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	fake_repository_TEST.cc \
	fake_installed_repository_TEST.cc \
	dep_parser_TEST.cc

fake_repository_TEST_SOURCES = \
	fake_repository_TEST.cc

fake_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

fake_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

fake_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

fake_installed_repository_TEST_SOURCES = \
	fake_installed_repository_TEST.cc

fake_installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

fake_installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

fake_installed_repository_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

dep_parser_TEST_SOURCES = \
	dep_parser_TEST.cc

dep_parser_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

dep_parser_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

dep_parser_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

noinst_LTLIBRARIES = libpaludisfakerepository.la

libpaludisfakerepository_la_SOURCES = \
	dep_parser.cc dep_parser.hh \
	fake_repository_base.cc fake_repository_base.hh \
	fake_repository.cc fake_repository.hh \
	fake_installed_repository.cc fake_installed_repository.hh \
	fake_package_id.cc fake_package_id.hh \
	registration.cc

paludis_repositories_fake_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/fake/
paludis_repositories_fake_include_HEADERS = \
	dep_parser.hh \
	fake_repository.hh \
	fake_repository_base.hh \
	fake_installed_repository.hh \
	fake_package_id.hh

