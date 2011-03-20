include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES = no_config_environment-se.hh no_config_environment-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisnoconfigenvironment_la_SOURCES = \
	no_config_environment.cc no_config_environment.hh \
	registration.cc

noinst_LTLIBRARIES = libpaludisnoconfigenvironment.la

paludis_environment_no_config_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/no_config
paludis_environment_no_config_include_HEADERS = \
	no_config_environment.hh \
	no_config_environment-se.hh

EXTRA_DIST = \
	no_config_environment-se.hh \
	no_config_environment-se.cc \
	no_config_environment.se \
	no_config_environment_TEST.cc \
	no_config_environment_TEST_setup.sh \
	no_config_environment_TEST_cleanup.sh

TESTS = no_config_environment_TEST

check_PROGRAMS = $(TESTS)
check_SCRIPTS = no_config_environment_TEST_setup.sh no_config_environment_TEST_cleanup.sh

no_config_environment_TEST_SOURCES = no_config_environment_TEST.cc

no_config_environment_TEST_LDADD = \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

no_config_environment_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

BUILT_SOURCES = \
	no_config_environment-se.hh \
	no_config_environment-se.cc

no_config_environment-se.hh : no_config_environment.se $(top_srcdir)/misc/make_se.bash
	$(top_srcdir)/misc/make_se.bash --header $(srcdir)/no_config_environment.se > $@

no_config_environment-se.cc : no_config_environment.se $(top_srcdir)/misc/make_se.bash
	$(top_srcdir)/misc/make_se.bash --source $(srcdir)/no_config_environment.se > $@

