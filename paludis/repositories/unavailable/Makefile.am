SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT
DISTCLEANFILES =

MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisunavailablerepository_@PALUDIS_PC_SLOT@.la

paludis_repositories_unavailable_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/unavailable/
libpaludisunavailablerepository_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_unavailable_include_HEADERS = \
	unavailable_repository.hh unavailable_repository-fwd.hh \
	unavailable_id.hh \
	unavailable_mask.hh \
	unavailable_repository_store.hh \
	unavailable_repository_file.hh unavailable_repository_file-fwd.hh

libpaludisunavailablerepository_@PALUDIS_PC_SLOT@_la_SOURCES = \
	unavailable_repository.cc \
	unavailable_id.cc \
	unavailable_mask.cc \
	unavailable_repository_store.cc \
	unavailable_repository_file.cc \
	registration.cc \
	$(paludis_repositories_unavailable_include_HEADERS)

libpaludisunavailablerepository_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

unavailable_repository_TEST_SOURCES = unavailable_repository_TEST.cc

unavailable_repository_TEST_LDADD = \
	libpaludisunavailablerepository_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

EXTRA_DIST = \
	unavailable_repository_TEST.cc \
	unavailable_repository_TEST_setup.sh \
	unavailable_repository_TEST_cleanup.sh

check_SCRIPTS = \
	unavailable_repository_TEST_setup.sh \
	unavailable_repository_TEST_cleanup.sh

TESTS_ENVIRONMENT = env \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_FETCHERS_DIR="$(top_srcdir)/paludis/fetchers/" \
	PALUDIS_OPTIONS="" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_CHOWN="yes" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = \
	unavailable_repository_TEST

check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

