include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

noinst_LTLIBRARIES = libpaludisvirtualsrepository.la

noinst_HEADERS = \
	virtuals_repository.hh \
	installed_virtuals_repository.hh

libpaludisvirtualsrepository_la_SOURCES = \
	virtuals_repository.cc \
	installed_virtuals_repository.cc \
	registration.cc \
	$(noinst_HEADERS)

TESTS =

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

EXTRA_DIST =

