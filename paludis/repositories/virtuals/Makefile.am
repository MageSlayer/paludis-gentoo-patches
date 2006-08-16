CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = vr_entry-sr.hh vr_entry-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisvirtualsrepository.la
paludis_repositories_virtuals_includedir = $(includedir)/paludis/repositories/virtuals/
libpaludisvirtualsrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_virtuals_include_HEADERS = \
	virtuals_repository.hh \
	installed_virtuals_repository.hh \
	vr_entry.hh

libpaludisvirtualsrepository_la_SOURCES = \
	virtuals_repository.cc \
	installed_virtuals_repository.cc \
	vr_entry.cc \
	$(paludis_repositories_portage_include_HEADERS)

EXTRA_DIST = \
	vr_entry-sr.hh \
	vr_entry-sr.cc \
	vr_entry.sr

BUILT_SOURCES = \
	vr_entry-sr.hh \
	vr_entry-sr.cc

TESTS =

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh

vr_entry-sr.hh : vr_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/vr_entry.sr > $@

vr_entry-sr.cc : vr_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/vr_entry.sr > $@



