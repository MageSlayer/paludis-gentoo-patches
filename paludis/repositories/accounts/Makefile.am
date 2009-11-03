include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

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

TESTS = \
	accounts_repository_TEST

check_PROGRAMS = $(TESTS)

