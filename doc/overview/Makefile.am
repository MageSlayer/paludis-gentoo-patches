SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	features.html \
	gettingstarted.html \
	contact.html \
	index.html

EXTRA_DIST = \
	toplinks.html.part.in \
	index.html.part \
	features.html.part \
	gettingstarted.html.part \
	contact.html.part

MAINTAINERCLEANFILES = Makefile.in

paludisoverviewhtmldir = $(htmldir)/overview

paludisoverviewhtml_DATA = \
	index.html \
	features.html \
	gettingstarted.html \
	contact.html

if HAVE_HTMLTIDY

TESTS = $(paludisoverviewhtml_DATA)
TESTS_ENVIRONMENT = bash $(top_srcdir)/doc/htmltidy.bash

endif

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

gettingstarted.html : header.html.part $(srcdir)/gettingstarted.html.part footer.html.part
	cat header.html.part $(srcdir)/gettingstarted.html.part footer.html.part > $@

features.html : header.html.part $(srcdir)/features.html.part footer.html.part
	cat header.html.part $(srcdir)/features.html.part footer.html.part > $@

contact.html : header.html.part $(srcdir)/contact.html.part footer.html.part
	cat header.html.part $(srcdir)/contact.html.part footer.html.part > $@

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

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisoverviewhtml_DATA)

