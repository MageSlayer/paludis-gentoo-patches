CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)
paludis_repositories_lib_LTLIBRARIES = libpaludisfakerepository.la
libpaludisfakerepository_la_SOURCES = fake_repository.cc fake_repository.hh
libpaludisfakerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
paludis_repositories_fake_includedir = $(includedir)/paludis/repositories/fake/
paludis_repositories_fake_include_HEADERS = fake_repository.hh

