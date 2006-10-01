CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = portage_repository-sr.hh portage_repository-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisportagerepository.la
paludis_repositories_portage_includedir = $(includedir)/paludis/repositories/portage/
libpaludisportagerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_portage_include_HEADERS = \
	portage_repository.hh \
	portage_repository_profile_file.hh \
	portage_repository_profile.hh \
	portage_repository_exceptions.hh \
	portage_repository_news.hh \
	portage_repository_params.hh \
	portage_repository_params-sr.hh \
	portage_repository_sets.hh \
	portage_repository_entries.hh \
	portage_repository_ebuild_entries.hh \
	portage_repository_ebin_entries.hh \
	make_ebuild_repository.hh \
	make_ebin_repository.hh

libpaludisportagerepository_la_SOURCES = \
	portage_repository.cc \
	portage_repository_profile_file.cc \
	portage_repository_profile.cc \
	portage_repository_exceptions.cc \
	portage_repository_news.cc \
	portage_repository_sets.cc \
	portage_repository_entries.cc \
	portage_repository_ebuild_entries.cc \
	portage_repository_ebin_entries.cc \
	portage_repository_params.cc \
	make_ebuild_repository.cc \
	make_ebin_repository.cc \
	$(paludis_repositories_portage_include_HEADERS)

libpaludisportagerepository_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

TESTS = portage_repository_TEST

portage_repository_TEST_SOURCES = portage_repository_TEST.cc

portage_repository_TEST_LDADD = \
	libpaludisportagerepository.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

portage_repository_TEST_CXXFLAGS = -I$(top_srcdir)

EXTRA_DIST = \
	portage_repository_TEST.cc \
	portage_repository_TEST_setup.sh \
	portage_repository_TEST_cleanup.sh \
	portage_repository_params-sr.hh \
	portage_repository_params-sr.cc \
	portage_repository_params.sr

BUILT_SOURCES = \
	portage_repository_params-sr.hh \
	portage_repository_params-sr.cc

check_PROGRAMS = $(TESTS)
check_SCRIPTS = portage_repository_TEST_setup.sh portage_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh

portage_repository_params-sr.hh : portage_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/portage_repository_params.sr > $@

portage_repository_params-sr.cc : portage_repository_params.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/portage_repository_params.sr > $@


