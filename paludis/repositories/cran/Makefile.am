SUBDIRS = cran .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

noinst_LTLIBRARIES = libpaludiscranrepository.la

noinst_HEADERS = \
	cran_package_id.hh \
	cran_dep_parser.hh \
	cran_repository.hh \
	cran_installed_repository.hh \
	description_file.hh \
	masks.hh \
	keys.hh \
	normalise.hh \
	dep_spec_pretty_printer.hh \
	package_dep_spec.hh

libpaludiscranrepository_la_SOURCES = \
	cran_package_id.cc \
	cran_dep_parser.cc \
	cran_repository.cc \
	cran_installed_repository.cc \
	description_file.cc \
	registration.cc \
	normalise.cc \
	masks.cc \
	keys.cc \
	dep_spec_pretty_printer.cc \
	package_dep_spec.cc \
	$(paludis_repositories_cran_include_HEADERS)

TESTS = \
	cran_dep_parser_TEST \
	cran_repository_TEST \
	cran_installed_repository_TEST \
	description_file_TEST

cran_dep_parser_TEST_SOURCES = cran_dep_parser_TEST.cc

cran_dep_parser_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

cran_dep_parser_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

cran_installed_repository_TEST_SOURCES = cran_installed_repository_TEST.cc

cran_installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

cran_installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

cran_repository_TEST_SOURCES = cran_repository_TEST.cc

cran_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

cran_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

description_file_TEST_SOURCES = description_file_TEST.cc

description_file_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

description_file_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

EXTRA_DIST = \
	cran_repository_TEST.cc \
	cran_repository_TEST_setup.sh \
	cran_repository_TEST_cleanup.sh \
	cran_installed_repository_TEST.cc \
	cran_installed_repository_TEST_setup.sh \
	cran_installed_repository_TEST_cleanup.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = \
	cran_repository_TEST_setup.sh cran_repository_TEST_cleanup.sh \
	cran_installed_repository_TEST_setup.sh cran_installed_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	CRAN_BASH_DIR="$(top_srcdir)/paludis/repositories/cran/cran/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

