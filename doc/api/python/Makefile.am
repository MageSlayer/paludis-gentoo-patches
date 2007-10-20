SUBDIRS = .

CLEANFILES = *~ epydoc/*
MAINTAINERCLEANFILES = Makefile.in

if ENABLE_PYTHON

epydoc : epydoc.css built-sources
	mkdir epydoc
	epydoc -n Paludis -o epydoc -u http://paludis.pioto.org --no-frames -c epydoc.css \
	    $(top_builddir)/python/paludis.so

else

epydoc :
	@echo "You don't have python turned on!"
	exit 1

endif


built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(DIST_SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapipythonhtml_DATA)

