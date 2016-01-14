include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DSHAREDIR=\"$(datarootdir)\"

libpaludispaludisenvironment_la_SOURCES = \
	bashable_conf.cc           bashable_conf.hh \
	extra_distribution_data.cc extra_distribution_data.hh \
	keywords_conf.cc           keywords_conf.hh \
	licenses_conf.cc           licenses_conf.hh \
	mirrors_conf.cc            mirrors_conf.hh \
	output_conf.cc             output_conf.hh \
	package_mask_conf.cc       package_mask_conf.hh \
	paludis_config.cc          paludis_config.hh \
	paludis_environment.cc     paludis_environment.hh \
	suggestions_conf.cc        suggestions_conf.hh \
	use_conf.cc                use_conf.hh \
	world.cc                   world.hh \
	registration.cc

shareenvpaludisdir = $(datarootdir)/paludis/environments/paludis/

dist_shareenvpaludis_DATA = \
	default_output.conf

dist_noinst_DATA = \
	tests_output.conf

libenvdir = $(libdir)/paludis/environments

noinst_LTLIBRARIES = libpaludispaludisenvironment.la

noinst_HEADERS = \
	bashable_conf.hh \
	extra_distribution_data.hh \
	keywords_conf.hh \
	licenses_conf.hh \
	mirrors_conf.hh \
	package_mask_conf.hh \
	paludis_config.hh \
	paludis_environment.hh \
	suggestions_conf.hh \
	use_conf.hh \
	world.hh

EXTRA_DIST = \
	paludis_environment_TEST_setup.sh \
	paludis_environment_TEST.cc \
	paludis_environment_TEST_cleanup.sh \
	world_TEST_setup.sh \
	world_TEST.cc \
	world_TEST_cleanup.sh \
	$(BUILT_SOURCES)

TESTS = paludis_environment_TEST world_TEST

check_PROGRAMS = $(TESTS)
check_SCRIPTS = \
	paludis_environment_TEST_setup.sh paludis_environment_TEST_cleanup.sh \
	world_TEST_setup.sh world_TEST_cleanup.sh

paludis_environment_TEST_SOURCES = paludis_environment_TEST.cc

paludis_environment_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

paludis_environment_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

paludis_environment_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

world_TEST_SOURCES = world_TEST.cc

world_TEST_LDADD = \
	$(top_builddir)/paludis/util/gtest_runner.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

world_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@ @GTESTDEPS_CXXFLAGS@

world_TEST_LDFLAGS = @GTESTDEPS_LDFLAGS@ @GTESTDEPS_LIBS@

logdir = @HACKED_LOG_DIR@

log_DATA = .keep

.keep :
	touch $@

CLEANFILES += .keep

