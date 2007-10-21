SUBDIRS = .

CLEANFILES = *~ python/*
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = epydoc.css

if ENABLE_PYTHON

all-local : epydoc

epydoc : python

python : epydoc.css
	rm -fr python
	mkdir -p python
	epydoc -n Paludis -o python -u http://paludis.pioto.org --no-frames -c epydoc.css \
	    $(top_builddir)/python/paludis.so

install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R python/ $(DESTDIR)$(htmldir)/api/

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

clean-local :
	rm -fr python/

