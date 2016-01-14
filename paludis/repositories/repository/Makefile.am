include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_builddir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

noinst_LTLIBRARIES = libpaludisrepositoryrepository.la

noinst_HEADERS = \
	repository_id.hh \
	repository_repository.hh repository_repository-fwd.hh \
	repository_repository_store.hh

libpaludisrepositoryrepository_la_SOURCES = \
	repository_id.cc \
	repository_repository.cc \
	repository_repository_store.cc \
	registration.cc \
	$(noinst_HEADERS)

