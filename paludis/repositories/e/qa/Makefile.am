SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@ \
	$(PCREPLUSPLUS_CFLAGS)

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

if ENABLE_QA

paludis_qa_datadir = $(datadir)/paludis/qa/

dist_paludis_qa_data_DATA = \
	extractors.conf \
	spec_keys_pds_blacklist.DEPEND.conf \
	spec_keys_pds_blacklist.RDEPEND.conf \
	spec_keys_pds_blacklist.PDEPEND.conf \
	spec_keys_pds_blacklist.SDEPEND.conf \
	spec_keys_pds_blacklist.PROVIDE.conf

paludis_repositories_e_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/e/
paludis_repositories_e_include_HEADERS = \
	eapi_supported.hh \
	metadata_keys.hh \
	extractors.hh \
	homepage_key.hh \
	spec_keys.hh \
	qa_checks.hh \
	qa_checks_group.hh \
	qa_controller.hh \
	short_description_key.hh \
	stray_files.hh \
	default_functions.hh \
	kv_variables.hh \
	restrict_key.hh \
	visibility.hh \
	whitespace.hh \
	header.hh \
	files_dir_size.hh \
	repo_name.hh

lib_LTLIBRARIES = libpaludiserepositoryqa.la

libpaludiserepositoryqa_la_SOURCES = \
	qa.cc \
	eapi_supported.cc \
	metadata_keys.cc \
	extractors.cc \
	homepage_key.cc \
	qa_checks.cc \
	qa_checks_group.cc \
	qa_controller.cc \
	short_description_key.cc \
	spec_keys.cc \
	stray_files.cc \
	default_functions.cc \
	kv_variables.cc \
	restrict_key.cc \
	visibility.cc \
	whitespace.cc \
	header.cc \
	files_dir_size.cc \
	repo_name.cc \
	$(paludis_repositories_e_include_HEADERS)

libpaludiserepositoryqa_la_LDFLAGS = \
	-version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0 \
	$(PCREPLUSPLUS_LIBS)

libpaludiserepositoryqa_la_LIBADD = \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

dist_check_SCRIPTS = \
	visibility_TEST_setup.sh \
	visibility_TEST_cleanup.sh

endif

EXTRA_DIST = test_extras.cc $(check_SCRIPTS)
BUILT_SOURCES =

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_QA_DATA_DIR="$(top_srcdir)/paludis/repositories/e/qa/" \
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
	extractors_TEST \
	homepage_key_TEST \
	spec_keys_TEST \
	stray_files_TEST \
	default_functions_TEST \
	kv_variables_TEST \
	visibility_TEST

check_PROGRAMS = $(TESTS)

spec_keys_TEST_SOURCES = spec_keys_TEST.cc
spec_keys_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

stray_files_TEST_SOURCES = stray_files_TEST.cc
stray_files_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

extractors_TEST_SOURCES = extractors_TEST.cc
extractors_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

homepage_key_TEST_SOURCES = homepage_key_TEST.cc
homepage_key_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

visibility_TEST_SOURCES = visibility_TEST.cc
visibility_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

default_functions_TEST_SOURCES = default_functions_TEST.cc
default_functions_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

kv_variables_TEST_SOURCES = kv_variables_TEST.cc
kv_variables_TEST_LDADD = \
	libpaludiserepositoryqa.la \
	test_extras.o \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/test/libtest.a

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

