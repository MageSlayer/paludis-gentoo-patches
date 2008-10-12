CLEANFILES = *~ *.epicfail
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
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(gtkmm_LIBS) $(gthread_LIBS) $(vte_LIBS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

