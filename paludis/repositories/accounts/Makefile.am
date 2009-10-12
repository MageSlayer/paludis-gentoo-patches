SUBDIRS = .
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
DISTCLEANFILES =

MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisaccountsrepository.la

noinst_HEADERS = \
	accounts_exceptions.hh \
	accounts_handler.hh \
	accounts_repository.hh \
	accounts_repository_store.hh \
	dummy_accounts_handler.hh \
	passwd_accounts_handler.hh \
	accounts_id.hh \
	accounts_dep_key.hh \
	accounts_installed_mask.hh \
	installed_accounts_id.hh

libpaludisaccountsrepository_la_SOURCES = \
	accounts_exceptions.cc \
	accounts_handler.cc \
	accounts_repository.cc \
	accounts_repository_store.cc \
	accounts_id.cc \
	accounts_dep_key.cc \
	accounts_installed_mask.cc \
	installed_accounts_id.cc \
	dummy_accounts_handler.cc \
	passwd_accounts_handler.cc \
	registration.cc \
	$(noinst_HEADERS)

accounts_repository_TEST_SOURCES = accounts_repository_TEST.cc

accounts_repository_TEST_LDADD = \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(DYNAMIC_LD_LIBS)

accounts_repository_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

EXTRA_DIST = \
	accounts_repository_TEST.cc

check_SCRIPTS =

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
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = \
	accounts_repository_TEST

check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done


