CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"


paludis_repositories_libdir = $(libdir)
paludis_repositories_lib_LTLIBRARIES = libpaludisportagerepository.la
paludis_repositories_portage_includedir = $(includedir)/paludis/repositories/portage/
libpaludisportagerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_portage_include_HEADERS = \
	portage_repository.hh \
	portage_repository_profile.hh \
	portage_repository_exceptions.hh

libpaludisportagerepository_la_SOURCES = \
	portage_repository.cc \
	portage_repository_profile.cc \
	portage_repository_exceptions.cc \
	$(paludis_repositories_portage_include_HEADERS)

TESTS = portage_repository_TEST

portage_repository_TEST_SOURCES = portage_repository_TEST.cc

portage_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisportagerepository.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

portage_repository_TEST_CXXFLAGS = -I$(top_srcdir)

EXTRA_DIST = \
	portage_repository_TEST.cc \
	portage_repository_TEST_setup.sh \
	portage_repository_TEST_cleanup.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = portage_repository_TEST_setup.sh portage_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

