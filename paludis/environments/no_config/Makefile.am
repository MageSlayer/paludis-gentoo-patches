CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda ihateautomake.cc ihateautomake.o
DISTCLEANFILES = no_config_environment-sr.hh no_config_environment-sr.cc
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisnoconfigenvironment_la_SOURCES = \
	no_config_environment.cc no_config_environment.hh

libpaludisnoconfigenvironment_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ! MONOLITHIC

libpaludisnoconfigenvironment_la_LIBADD = \
	$(top_builddir)/paludis/repositories/gentoo/libpaludisgentoorepository.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

endif

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisnoconfigenvironment.la

else

lib_LTLIBRARIES = libpaludisnoconfigenvironment.la

endif

paludis_environment_no_config_includedir = $(includedir)/paludis/environments/no_config
paludis_environment_no_config_include_HEADERS = \
	no_config_environment.hh \
	no_config_environment-sr.hh

EXTRA_DIST = \
	no_config_environment-sr.hh \
	no_config_environment-sr.cc \
	no_config_environment.sr

BUILT_SOURCES = \
	no_config_environment-sr.hh \
	no_config_environment-sr.cc

no_config_environment-sr.hh : no_config_environment.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/no_config_environment.sr > $@

no_config_environment-sr.cc : no_config_environment.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/no_config_environment.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

