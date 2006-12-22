CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"

paludis_repositories_libdir = $(libdir)/paludis/repositories

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisfakerepository.la

else

paludis_repositories_lib_LTLIBRARIES = libpaludisfakerepository.la

endif

libpaludisfakerepository_la_SOURCES = \
	fake_repository_base.cc fake_repository_base.hh \
	fake_repository.cc fake_repository.hh \
	fake_installed_repository.cc fake_installed_repository.hh

libpaludisfakerepository_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DYNAMIC_LD_LIBS)

libpaludisfakerepository_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

paludis_repositories_fake_includedir = $(includedir)/paludis/repositories/fake/
paludis_repositories_fake_include_HEADERS = \
	fake_repository.hh \
	fake_repository_base.hh \
	fake_installed_repository.hh

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

