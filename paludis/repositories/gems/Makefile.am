CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = gems_repository-sr.hh gems_repository-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisgemsrepository.la
paludis_repositories_gems_includedir = $(includedir)/paludis/repositories/gems/
libpaludisgemsrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_gems_include_HEADERS = \
	gems_repository.hh \
	gems_repository-sr.hh \
	gems_repository_exceptions.hh \
	make_gems_repository.hh

libpaludisgemsrepository_la_SOURCES = \
	gems_repository.cc \
	gems_repository_exceptions.cc \
	make_gems_repository.cc \
	$(paludis_repositories_gems_include_HEADERS)

libpaludisgemsrepository_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(DYNAMIC_LD_LIBS)

gems_repository_TEST_SOURCES = gems_repository_TEST.cc

gems_repository_TEST_LDADD = \
	libpaludisgemsrepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

gems_repository_TEST_CXXFLAGS = -I$(top_srcdir)

EXTRA_DIST = \
	gems_repository_TEST.cc \
	gems_repository_TEST_setup.sh \
	gems_repository_TEST_cleanup.sh \
	gems_repository.sr \
	gems_repository-sr.hh \
	gems_repository-sr.cc

BUILT_SOURCES = \
	gems_repository-sr.hh \
	gems_repository-sr.cc

check_SCRIPTS = \
	gems_repository_TEST_setup.sh gems_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gems/`" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = gems_repository_TEST
check_PROGRAMS = $(TESTS)

gems_repository-sr.hh : gems_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/gems_repository.sr > $@

gems_repository-sr.cc : gems_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/gems_repository.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done



