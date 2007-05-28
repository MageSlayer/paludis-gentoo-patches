CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda ihateautomake.cc ihateautomake.o
DISTCLEANFILES = no_config_environment-sr.hh no_config_environment-sr.cc
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisnoconfigenvironment_la_SOURCES = \
	no_config_environment.cc no_config_environment.hh \
	registration.cc

libpaludisnoconfigenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ! MONOLITHIC

libpaludisnoconfigenvironment_la_LIBADD = \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

endif

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisnoconfigenvironment.la

else

lib_LTLIBRARIES = libpaludisnoconfigenvironment.la

endif

paludis_environment_no_config_includedir = $(includedir)/paludis/environments/no_config
paludis_environment_no_config_include_HEADERS = \
	no_config_environment.hh \
	no_config_environment-sr.hh

EXTRA_DIST = \
	no_config_environment-sr.hh \
	no_config_environment-sr.cc \
	no_config_environment.sr \
	no_config_environment_TEST.cc \
	no_config_environment_TEST_setup.sh \
	no_config_environment_TEST_cleanup.sh

TESTS = no_config_environment_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = no_config_environment_TEST_setup.sh no_config_environment_TEST_cleanup.sh

no_config_environment_TEST_SOURCES = no_config_environment_TEST.cc

no_config_environment_TEST_LDADD = \
	libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

BUILT_SOURCES = \
	no_config_environment-sr.hh \
	no_config_environment-sr.cc

no_config_environment-sr.hh : no_config_environment.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/no_config_environment.sr > $@

no_config_environment-sr.cc : no_config_environment.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/no_config_environment.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

