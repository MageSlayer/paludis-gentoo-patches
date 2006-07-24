CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)
paludis_repositories_lib_LTLIBRARIES = libpaludisnothingrepository.la
libpaludisnothingrepository_la_SOURCES = nothing_repository.cc nothing_repository.hh
libpaludisnothingrepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
paludis_repositories_nothing_includedir = $(includedir)/paludis/repositories/nothing/
paludis_repositories_nothing_include_HEADERS = nothing_repository.hh

