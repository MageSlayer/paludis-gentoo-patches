CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = \
	use_config_entry-sr.hh use_config_entry-sr.cc \
	repository_config_entry-sr.hh repository_config_entry-sr.cc
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisdefaultenvironment_la_SOURCES = \
	default_config.cc      default_config.hh \
	default_environment.cc default_environment.hh

libpaludisdefaultenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
libpaludisdefaultenvironment_la_LIBADD = \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisdefaultenvironment.la

else

lib_LTLIBRARIES = libpaludisdefaultenvironment.la

endif

paludis_environment_default_includedir = $(includedir)/paludis/environment/default
paludis_environment_default_include_HEADERS = \
	default_config.hh \
	default_environment.hh \
	use_config_entry-sr.hh \
	repository_config_entry-sr.hh

EXTRA_DIST = \
	default_environment_TEST_setup.sh \
	default_environment_TEST.cc \
	default_environment_TEST_cleanup.sh \
	$(BUILT_SOURCES) \
	use_config_entry.sr \
	repository_config_entry.sr

TESTS = default_environment_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = default_environment_TEST_setup.sh default_environment_TEST_cleanup.sh

default_environment_TEST_SOURCES = default_environment_TEST.cc
default_environment_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	libpaludisdefaultenvironment.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

use_config_entry-sr.hh : use_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/use_config_entry.sr > $@

use_config_entry-sr.cc : use_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/use_config_entry.sr > $@

repository_config_entry-sr.hh : repository_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/repository_config_entry.sr > $@

repository_config_entry-sr.cc : repository_config_entry.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/repository_config_entry.sr > $@

