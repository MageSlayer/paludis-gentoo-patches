SUBDIRS = cplusplus python ruby .

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html

EXTRA_DIST = \
	toplinks.html.part.in \
	index.html.part

MAINTAINERCLEANFILES = Makefile.in

paludisapihtmldir = $(htmldir)/api

paludisapihtml_DATA = \
	index.html

if HAVE_HTMLTIDY

TESTS = $(paludisapihtml_DATA)
TESTS_ENVIRONMENT = bash $(top_srcdir)/doc/htmltidy.bash

endif

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

header.html.part : $(srcdir)/../header.html.part.in toplinks.html.part
	sed \
	    -e 's,###TOPURI###,../,g' \
	    -e '/###TOPLINKS###/r $(srcdir)/toplinks.html.part' \
	    -e 's,###TOPLINKS###,,g' \
	    < $(srcdir)/../header.html.part.in > header.html.part

toplinks.html.part : $(srcdir)/toplinks.html.part.in
	sed \
	    -e 's,###TOPURI###,../,g' \
	    < $(srcdir)/toplinks.html.part.in > toplinks.html.part

footer.html.part : $(srcdir)/../footer.html.part.in
	sed -e 's,###TOPURI###,./,g' < $(srcdir)/../footer.html.part.in > footer.html.part


built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

doxygen :
	$(MAKE) -C cplusplus $@

epydoc :
	$(MAKE) -C python $@

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapihtml_DATA)

