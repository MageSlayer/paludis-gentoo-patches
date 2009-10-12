SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@ \
	$(PCRECPPDEPS_CFLAGS)

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

if ENABLE_QA

paludis_qa_datadir = $(datadir)/paludis/qa/

dist_paludis_qa_data_DATA = \
	deprecated_functions.conf \
	extractors.conf \
	inherited_blacklist.conf \
	iuse_blacklist.conf \
	spec_keys_pds_blacklist.DEPEND.conf \
	spec_keys_pds_blacklist.RDEPEND.conf \
	spec_keys_pds_blacklist.PDEPEND.conf \
	spec_keys_pds_blacklist.SDEPEND.conf \
	spec_keys_pds_blacklist.PROVIDE.conf

noinst_HEADERS = \
	eapi_supported.hh \
	metadata_keys.hh \
	extractors.hh \
	fetches_key.hh \
	homepage_key.hh \
	iuse_key.hh \
	keywords_key.hh \
	license_key.hh \
	spec_keys.hh \
	qa_checks.hh \
	qa_checks_group.hh \
	qa_controller.hh \
	short_description_key.hh \
	stray_files.hh \
	default_functions.hh \
	kv_variables.hh \
	root_variable.hh \
	subshell_die.hh \
	restrict_key.hh \
	inherited_key.hh \
	visibility.hh \
	function_keyword.hh \
	whitespace.hh \
	header.hh \
	variable_assigns.hh \
	deprecated_functions.hh \
	manifest.hh \
	ebuild_count.hh \
	changelog.hh \
	misc_files.hh \
	files_dir_size.hh \
	repo_name.hh \
	categories.hh

lib_LTLIBRARIES = libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la

libpaludiserepositoryqa_@PALUDIS_PC_SLOT@_la_LDFLAGS = \
	$(PCRECPPDEPS_LIBS)

libpaludiserepositoryqa_@PALUDIS_PC_SLOT@_la_SOURCES = \
	qa.cc \
	eapi_supported.cc \
	metadata_keys.cc \
	extractors.cc \
	fetches_key.cc \
	homepage_key.cc \
	iuse_key.cc \
	keywords_key.cc \
	license_key.cc \
	qa_checks.cc \
	qa_checks_group.cc \
	qa_controller.cc \
	short_description_key.cc \
	spec_keys.cc \
	stray_files.cc \
	default_functions.cc \
	kv_variables.cc \
	root_variable.cc \
	subshell_die.cc \
	restrict_key.cc \
	inherited_key.cc \
	visibility.cc \
	function_keyword.cc \
	whitespace.cc \
	header.cc \
	variable_assigns.cc \
	deprecated_functions.cc \
	manifest.cc \
	ebuild_count.cc \
	changelog.cc \
	misc_files.cc \
	files_dir_size.cc \
	repo_name.cc \
	categories.cc \
	$(paludis_repositories_e_include_HEADERS)

dist_check_SCRIPTS = \
	visibility_TEST_setup.sh \
	visibility_TEST_cleanup.sh \
	misc_files_TEST_setup.sh \
	misc_files_TEST_cleanup.sh \
	manifest_TEST_setup.sh \
	manifest_TEST_cleanup.sh

endif

EXTRA_DIST = $(check_SCRIPTS)
BUILT_SOURCES =

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OPTIONS="" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_SUFFIXES_FILE="$(top_srcdir)/paludis/repositories/e/ebuild_entries_suffixes.conf" \
	PALUDIS_QA_DATA_DIR="$(top_srcdir)/paludis/repositories/e/qa/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
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
	default_functions_TEST \
	kv_variables_TEST \
	visibility_TEST \
	misc_files_TEST \
	manifest_TEST \
	subshell_die_TEST \
	function_keyword_TEST

check_PROGRAMS = $(TESTS)

spec_keys_TEST_SOURCES = spec_keys_TEST.cc
spec_keys_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

spec_keys_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

extractors_TEST_SOURCES = extractors_TEST.cc
extractors_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

extractors_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

homepage_key_TEST_SOURCES = homepage_key_TEST.cc
homepage_key_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

homepage_key_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

visibility_TEST_SOURCES = visibility_TEST.cc
visibility_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

visibility_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

default_functions_TEST_SOURCES = default_functions_TEST.cc
default_functions_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

default_functions_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

kv_variables_TEST_SOURCES = kv_variables_TEST.cc
kv_variables_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

kv_variables_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

misc_files_TEST_SOURCES = misc_files_TEST.cc
misc_files_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

misc_files_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

manifest_TEST_SOURCES = manifest_TEST.cc
manifest_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

manifest_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

subshell_die_TEST_SOURCES = subshell_die_TEST.cc
subshell_die_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

subshell_die_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

function_keyword_TEST_SOURCES = function_keyword_TEST.cc
function_keyword_TEST_LDADD = \
	libpaludiserepositoryqa_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

function_keyword_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

