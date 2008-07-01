CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT
DISTCLEANFILES = \
	use_config_entry-sr.hh use_config_entry-sr.cc
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludispaludisenvironment_la_SOURCES = \
	bashable_conf.cc       bashable_conf.hh \
	keywords_conf.cc       keywords_conf.hh \
	licenses_conf.cc       licenses_conf.hh \
	package_mask_conf.cc   package_mask_conf.hh \
	use_conf.cc            use_conf.hh \
	mirrors_conf.cc        mirrors_conf.hh \
	world.cc               world.hh \
	paludis_config.cc      paludis_config.hh \
	paludis_environment.cc paludis_environment.hh \
	registration.cc

libpaludispaludisenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ! MONOLITHIC

libpaludispaludisenvironment_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

endif

libenvdir = $(libdir)/paludis/environments/

if MONOLITHIC

noinst_LTLIBRARIES = libpaludispaludisenvironment.la

else

libenv_LTLIBRARIES = libpaludispaludisenvironment.la

endif

paludis_environment_paludis_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/paludis
paludis_environment_paludis_include_HEADERS = \
	paludis_config.hh \
	paludis_environment.hh \
	use_config_entry-sr.hh \
	use_conf.hh \
	keywords_conf.hh \
	licenses_conf.hh \
	bashable_conf.hh \
	package_mask_conf.hh \
	world.hh \
	mirrors_conf.hh

EXTRA_DIST = \
	paludis_environment_TEST_setup.sh \
	paludis_environment_TEST.cc \
	paludis_environment_TEST_cleanup.sh \
	$(BUILT_SOURCES) \
	use_config_entry.sr

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
	PALUDIS_FETCHERS_DIR="$(top_srcdir)/paludis/fetchers/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_CHOWN="yes" \
	PALUDIS_TESTS_KEEP_STDERR="yes" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = paludis_environment_TEST_setup.sh paludis_environment_TEST_cleanup.sh

paludis_environment_TEST_SOURCES = paludis_environment_TEST.cc

if MONOLITHIC

paludis_environment_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

else

paludis_environment_TEST_LDADD = \
	libpaludispaludisenvironment.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

use_config_entry-sr.hh : use_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/use_config_entry.sr > $@

use_config_entry-sr.cc : use_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/use_config_entry.sr > $@

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

