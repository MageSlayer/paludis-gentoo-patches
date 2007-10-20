CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisvirtualsrepository.la

else

paludis_repositories_lib_LTLIBRARIES = libpaludisvirtualsrepository.la

endif

paludis_repositories_virtuals_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/virtuals/
libpaludisvirtualsrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_virtuals_include_HEADERS = \
	virtuals_repository.hh \
	installed_virtuals_repository.hh \
	package_id.hh

libpaludisvirtualsrepository_la_SOURCES = \
	virtuals_repository.cc \
	installed_virtuals_repository.cc \
	package_id.cc \
	registration.cc \
	$(paludis_repositories_portage_include_HEADERS)

if ! MONOLITHIC

libpaludisvirtualsrepository_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

endif

TESTS = virtuals_repository_TEST installed_virtuals_repository_TEST

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

EXTRA_DIST = $(virtuals_repository_TEST_SOURCES) $(installed_virtuals_repository_TEST_SOURCES)

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

virtuals_repository_TEST_LDADD = \
	libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/test/libtest.a

installed_virtuals_repository_TEST_LDADD = \
	libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/test/libtest.a

virtuals_repository_TEST_SOURCES = \
	virtuals_repository_TEST.cc

installed_virtuals_repository_TEST_SOURCES = \
	installed_virtuals_repository_TEST.cc

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
