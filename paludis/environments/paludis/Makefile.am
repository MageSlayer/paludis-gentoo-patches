SUBDIRS = output_managers .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DSHAREDIR=\"$(datarootdir)\"

libpaludispaludisenvironment_la_SOURCES = \
	action_to_string.cc    action_to_string.hh \
	bashable_conf.cc       bashable_conf.hh \
	keywords_conf.cc       keywords_conf.hh \
	licenses_conf.cc       licenses_conf.hh \
	package_mask_conf.cc   package_mask_conf.hh \
	use_conf.cc            use_conf.hh \
	mirrors_conf.cc        mirrors_conf.hh \
	output_conf.cc         output_conf.hh \
	output_managers.cc     output_managers.hh \
	world.cc               world.hh \
	paludis_config.cc      paludis_config.hh \
	paludis_environment.cc paludis_environment.hh \
	extra_distribution_data.cc extra_distribution_data.hh \
	registration.cc

shareenvpaludisdir = $(datarootdir)/paludis/environments/paludis/

dist_shareenvpaludis_DATA = \
	default_output.conf \
	tests_output.conf

libenvdir = $(libdir)/paludis/environments

noinst_LTLIBRARIES = libpaludispaludisenvironment.la

noinst_HEADERS = \
	paludis_config.hh \
	paludis_environment.hh \
	use_conf.hh \
	keywords_conf.hh \
	licenses_conf.hh \
	bashable_conf.hh \
	package_mask_conf.hh \
	world.hh \
	mirrors_conf.hh \
	extra_distribution_data.hh

EXTRA_DIST = \
	paludis_environment_TEST_setup.sh \
	paludis_environment_TEST.cc \
	paludis_environment_TEST_cleanup.sh \
	$(BUILT_SOURCES)

TESTS = paludis_environment_TEST

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_SUFFIXES_FILE="$(top_srcdir)/paludis/repositories/e/ebuild_entries_suffixes.conf" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_FETCHERS_DIR="$(top_srcdir)/paludis/fetchers/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_CHOWN="yes" \
	PALUDIS_TESTS_KEEP_STDERR="yes" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = paludis_environment_TEST_setup.sh paludis_environment_TEST_cleanup.sh

paludis_environment_TEST_SOURCES = paludis_environment_TEST.cc

paludis_environment_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

paludis_environment_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

