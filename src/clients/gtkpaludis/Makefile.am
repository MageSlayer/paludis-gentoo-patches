CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
SUBDIRS = vtemm libgtkpaludis .

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src/clients/gtkpaludis \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@ \
	      $(gtkmm_CFLAGS) $(gthread_CFLAGS) \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ @PALUDIS_CXXFLAGS_NO_WSHADOW@

bin_PROGRAMS = gtkpaludis

TESTS =

gtkpaludis_SOURCES = \
	gtkpaludis.cc

gtkpaludis_LDADD = \
	libgtkpaludis/libgtkpaludis.a \
	vtemm/libvtemm.a \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(gtkmm_LIBS) $(gthread_LIBS) $(vte_LIBS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

