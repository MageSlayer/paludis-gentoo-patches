SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = \
	installed_repository-sr.hh installed_repository-sr.cc

MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisunpackagedrepository.la

paludis_repositories_unpackaged_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/unpackaged/
libpaludisunpackagedrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_unpackaged_include_HEADERS = \
	dep_parser.hh \
	dep_printer.hh \
	exceptions.hh \
	unpackaged_id.hh \
	unpackaged_repository.hh \
	unpackaged_key.hh \
	unpackaged_stripper.hh \
	installed_repository.hh \
	installed_repository-sr.hh \
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
	$(paludis_repositories_unpackaged_include_HEADERS)

libpaludisunpackagedrepository_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

unpackaged_repository_TEST_SOURCES = unpackaged_repository_TEST.cc

unpackaged_repository_TEST_LDADD = \
	libpaludisunpackagedrepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

installed_repository_TEST_SOURCES = installed_repository_TEST.cc

installed_repository_TEST_LDADD = \
	libpaludisunpackagedrepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

dep_parser_TEST_SOURCES = dep_parser_TEST.cc

dep_parser_TEST_LDADD = \
	libpaludisunpackagedrepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

EXTRA_DIST = \
	unpackaged_repository_TEST.cc \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST.cc \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh \
	installed_repository.sr \
	installed_repository-sr.hh \
	installed_repository-sr.cc \
	installed_id.cc \
	dep_parser_TEST.cc

BUILT_SOURCES = \
	installed_repository-sr.hh installed_repository-sr.cc

check_SCRIPTS = \
	unpackaged_repository_TEST_setup.sh \
	unpackaged_repository_TEST_cleanup.sh \
	installed_repository_TEST_setup.sh \
	installed_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_FETCHERS_DIR="$(top_srcdir)/paludis/fetchers/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_CHOWN="yes" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = \
	unpackaged_repository_TEST \
	installed_repository_TEST \
	dep_parser_TEST

check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

installed_repository-sr.hh : installed_repository.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $(srcdir)/installed_repository.sr > $@ ; then rm -f $@ ; exit 1 ; fi

installed_repository-sr.cc : installed_repository.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $(srcdir)/installed_repository.sr > $@ ; then rm -f $@ ; exit 1 ; fi

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

