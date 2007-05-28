SUBDIRS = cran .
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = cran_repository-sr.hh cran_repository-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

if MONOLITHIC

noinst_LTLIBRARIES = libpaludiscranrepository.la

else

paludis_repositories_lib_LTLIBRARIES = libpaludiscranrepository.la

endif

paludis_repositories_cran_includedir = $(includedir)/paludis/repositories/cran/
libpaludiscranrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_cran_include_HEADERS = \
	cran_version_metadata.hh \
	cran_description.hh \
	cran_dep_parser.hh \
	cran_repository.hh \
	cran_repository-sr.hh \
	cran_installed_repository.hh \
	cran_installed_repository-sr.hh

libpaludiscranrepository_la_SOURCES = \
	cran_version_metadata.cc \
	cran_description.cc \
	cran_dep_parser.cc \
	cran_repository.cc \
	cran_installed_repository.cc \
	registration.cc \
	$(paludis_repositories_cran_include_HEADERS)

if ! MONOLITHIC

libpaludiscranrepository_la_LIBADD = \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

endif

TESTS = cran_dep_parser_TEST cran_repository_TEST

cran_dep_parser_TEST_SOURCES = cran_dep_parser_TEST.cc

cran_dep_parser_TEST_LDADD= \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludiscranrepository.la

cran_repository_TEST_SOURCES = cran_repository_TEST.cc

cran_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludiscranrepository.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(DYNAMIC_LD_LIBS)

cran_repository_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS)

EXTRA_DIST = \
	cran_repository_TEST.cc \
	cran_repository_TEST_setup.sh \
	cran_repository_TEST_cleanup.sh \
	cran_repository.sr \
	cran_repository-sr.hh \
	cran_repository-sr.cc \
	cran_installed_repository.sr \
	cran_installed_repository-sr.hh \
	cran_installed_repository-sr.cc

BUILT_SOURCES = \
	cran_repository-sr.hh \
	cran_repository-sr.cc \
	cran_installed_repository-sr.hh \
	cran_installed_repository-sr.cc

check_PROGRAMS = $(TESTS)
check_SCRIPTS = cran_repository_TEST_setup.sh cran_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	CRAN_BASH_DIR="$(top_srcdir)/paludis/repositories/cran/cran/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh

cran_repository-sr.hh : cran_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/cran_repository.sr > $@

cran_repository-sr.cc : cran_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/cran_repository.sr > $@

cran_installed_repository-sr.hh : cran_installed_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/cran_installed_repository.sr > $@

cran_installed_repository-sr.cc : cran_installed_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/cran_installed_repository.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

