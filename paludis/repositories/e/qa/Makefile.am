SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

if ENABLE_QA

paludis_repositories_e_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/e/
paludis_repositories_e_include_HEADERS = \
	eapi_supported.hh \
	homepage_key.hh \
	spec_keys.hh \
	qa_checks.hh \
	qa_checks_group.hh \
	qa_controller.hh \
	short_description_key.hh \
	stray_files.hh

noinst_LTLIBRARIES = libpaludiserepositoryqa.la

libpaludiserepositoryqa_la_SOURCES = \
	eapi_supported.cc \
	homepage_key.cc \
	qa_checks.cc \
	qa_checks_group.cc \
	qa_controller.cc \
	short_description_key.cc \
	spec_keys.cc \
	stray_files.cc \
	$(paludis_repositories_e_include_HEADERS)

endif

EXTRA_DIST =
BUILT_SOURCES =

check_SCRIPTS =

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

if ENABLE_QA

TESTS = \
	spec_keys_TEST \
	stray_files_TEST

check_PROGRAMS = $(TESTS)

spec_keys_TEST_SOURCES = spec_keys_TEST.cc
spec_keys_TEST_LDFLAGS = \
	libpaludiserepositoryqa.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

stray_files_TEST_SOURCES = stray_files_TEST.cc
stray_files_TEST_LDFLAGS = \
	libpaludiserepositoryqa.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

