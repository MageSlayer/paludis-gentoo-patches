include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_gems_libexecdir = $(libexecdir)/paludis/gems

noinst_LTLIBRARIES = libpaludisgemsrepository.la

noinst_HEADERS = \
	gems_repository.hh \
	installed_gems_repository.hh \
	exceptions.hh \
	yaml.hh yaml-fwd.hh \
	gem_specification.hh gem_specification-fwd.hh \
	gem_specifications.hh \
	params-fwd.hh params.hh \
	extra_distribution_data.hh

libpaludisgemsrepository_la_SOURCES = \
	gems_repository.cc \
	installed_gems_repository.cc \
	params.cc \
	exceptions.cc \
	yaml.cc \
	gem_specification.cc \
	gem_specifications.cc \
	extra_distribution_data.cc \
	registration.cc \
	$(noinst_HEADERS)

libpaludisgemsrepository_la_LIBADD = \
	-lsyck

gems_repository_TEST_SOURCES = gems_repository_TEST.cc
installed_gems_repository_TEST_SOURCES = gems_repository_TEST.cc
yaml_TEST_SOURCES = yaml_TEST.cc
gem_specification_TEST_SOURCES = gem_specification_TEST.cc
gem_specifications_TEST_SOURCES = gem_specifications_TEST.cc

gems_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	-lsyck

installed_gems_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	-lsyck

yaml_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	-lsyck

gem_specification_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	-lsyck

gem_specifications_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	-lsyck

gems_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir) @PALUDIS_CXXFLAGS_NO_DEBUGGING@
installed_gems_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir) @PALUDIS_CXXFLAGS_NO_DEBUGGING@
yaml_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir) @PALUDIS_CXXFLAGS_NO_DEBUGGING@
gem_specification_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir) @PALUDIS_CXXFLAGS_NO_DEBUGGING@
gem_specifications_TEST_CXXFLAGS = $(AM_CXXFLAGS) -I$(top_srcdir) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

EXTRA_DIST = \
	installed_gems_repository_TEST.cc \
	installed_gems_repository_TEST_setup.sh \
	installed_gems_repository_TEST_cleanup.sh \
	gems_repository_TEST.cc \
	gems_repository_TEST_setup.sh \
	gems_repository_TEST_cleanup.sh \
	yaml_TEST.cc \
	gem_specification_TEST.cc \
	gem_specifications_TEST.cc

check_SCRIPTS = \
	gems_repository_TEST_setup.sh gems_repository_TEST_cleanup.sh

paludis_gems_libexec_SCRIPTS = gems.bash

TESTS = gems_repository_TEST installed_gems_repository_TEST yaml_TEST gem_specification_TEST gem_specifications_TEST
check_PROGRAMS = $(TESTS)

