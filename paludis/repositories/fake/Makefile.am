CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

TESTS_ENVIRONMENT = env \
	PALUDIS_GEMS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gems/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gems/`:` \
		$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gems/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = fake_repository_TEST fake_installed_repository_TEST dep_parser_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	fake_repository_TEST.cc \
	fake_installed_repository_TEST.cc \
	dep_parser_TEST.cc

fake_repository_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

fake_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

fake_installed_repository_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

fake_installed_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

dep_parser_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a

dep_parser_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

fake_repository_TEST_SOURCES = \
	fake_repository_TEST.cc

fake_installed_repository_TEST_SOURCES = \
	fake_installed_repository_TEST.cc

dep_parser_TEST_SOURCES = \
	dep_parser_TEST.cc

noinst_LTLIBRARIES = libpaludisfakerepository.la

libpaludisfakerepository_la_SOURCES = \
	dep_parser.cc dep_parser.hh \
	fake_repository_base.cc fake_repository_base.hh \
	fake_repository.cc fake_repository.hh \
	fake_installed_repository.cc fake_installed_repository.hh \
	fake_package_id.cc fake_package_id.hh \
	registration.cc

paludis_repositories_fake_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/fake/
paludis_repositories_fake_include_HEADERS = \
	dep_parser.hh \
	fake_repository.hh \
	fake_repository_base.hh \
	fake_installed_repository.hh \
	fake_package_id.hh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

