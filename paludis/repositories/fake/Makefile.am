CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

TESTS_ENVIRONMENT = env \
	PALUDIS_GEMS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gems/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gems/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gems/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = fake_repository_TEST fake_installed_repository_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	fake_repository_TEST.cc \
	fake_installed_repository_TEST.cc \
	fake_repository-sr.hh \
	fake_repository-sr.cc

fake_repository_TEST_LDADD = \
	libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/test/libtest.a

fake_installed_repository_TEST_LDADD = \
	libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/test/libtest.a

fake_repository_TEST_SOURCES = \
	fake_repository_TEST.cc

fake_installed_repository_TEST_SOURCES = \
	fake_installed_repository_TEST.cc

paludis_repositories_lib_LTLIBRARIES = libpaludisfakerepository.la

libpaludisfakerepository_la_SOURCES = \
	fake_repository_base.cc fake_repository_base.hh \
	fake_repository.cc fake_repository.hh \
	fake_installed_repository.cc fake_installed_repository.hh \
	fake_package_id.cc fake_package_id.hh

libpaludisfakerepository_la_LIBADD = \
	$(top_builddir)/paludis/repositories/e/libpaludiserepository.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

libpaludisfakerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_fake_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/fake/
paludis_repositories_fake_include_HEADERS = \
	fake_repository.hh \
	fake_repository_base.hh \
	fake_installed_repository.hh \
	fake_package_id.hh \
	fake_repository-sr.hh

BUILT_SOURCES = \
	fake_repository-sr.hh \
	fake_repository-sr.cc

fake_repository-sr.hh : fake_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/fake_repository.sr > $@

fake_repository-sr.cc : fake_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/fake_repository.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

