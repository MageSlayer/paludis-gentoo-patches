CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES =
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisportageenvironment_la_SOURCES = \
	registration.cc \
	portage_environment.cc portage_environment.hh

libpaludisportageenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisportageenvironment_la_LIBADD = \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

libenvdir = $(libdir)/paludis/environments/

libexecenvdir = $(libexecdir)/paludis/environments/portage/
libexecenv_SCRIPTS = bashrc

libenv_LTLIBRARIES = libpaludisportageenvironment.la

paludis_environment_portage_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/environments/portage
paludis_environment_portage_include_HEADERS = \
	portage_environment.hh

EXTRA_DIST = \
	$(BUILT_SOURCES) \
	$(libexecenv_SCRIPTS) \
	portage_environment_TEST_setup.sh \
	portage_environment_TEST.cc \
	portage_environment_TEST_cleanup.sh

TESTS = portage_environment_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh

portage_environment_TEST_SOURCES = portage_environment_TEST.cc
portage_environment_TEST_LDADD = \
	libpaludisportageenvironment.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

check_PROGRAMS = $(TESTS)
check_SCRIPTS = portage_environment_TEST_setup.sh portage_environment_TEST_cleanup.sh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

