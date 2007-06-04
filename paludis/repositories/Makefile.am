if MONOLITHIC

SUBDIRS = $(BUILD_REPOSITORIES) .

else

SUBDIRS = . $(BUILD_REPOSITORIES)

endif

DIST_SUBDIRS = $(ALL_REPOSITORIES)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

lib_LTLIBRARIES = libpaludisrepositories.la

paludis_repositories_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/repositories/

paludis_repositories_include_HEADERS = \
	repository_maker.hh

libpaludisrepositories_la_SOURCES = \
	repository_maker.cc \
	$(paludis_repositories_include_HEADERS)

libpaludisrepositories_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if MONOLITHIC

libpaludisrepositories_la_LIBADD = \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/paludis/repositories/vdb/libpaludisvdbrepository.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	@DYNAMIC_LD_LIBS@

else

libpaludisrepositories_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	@DYNAMIC_LD_LIBS@

endif

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

