CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = nothing_repository-sr.hh nothing_repository-sr.cc
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories
paludis_repositories_lib_LTLIBRARIES = libpaludisnothingrepository.la
libpaludisnothingrepository_la_SOURCES = nothing_repository.cc nothing_repository.hh
libpaludisnothingrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
paludis_repositories_nothing_includedir = $(includedir)/paludis/repositories/nothing/

paludis_repositories_nothing_include_HEADERS = \
	nothing_repository.hh \
	nothing_repository-sr.hh

EXTRA_DIST = \
	nothing_repository-sr.hh \
	nothing_repository-sr.cc \
	nothing_repository.sr

BUILT_SOURCES = \
	nothing_repository-sr.hh \
	nothing_repository-sr.cc

nothing_repository-sr.hh : nothing_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/nothing_repository.sr > $@

nothing_repository-sr.cc : nothing_repository.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/nothing_repository.sr > $@

