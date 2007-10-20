SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html \
	different.html \
	general.html \
	howdoi.html \
	misfunctionality.html \
	operation.html \
	stricter.html \
	repositories.html

EXTRA_DIST = \
	toplinks.html.part.in \
	index.html.part \
	different.html.part \
	general.html.part \
	howdoi.html.part \
	misfunctionality.html.part \
	operation.html.part \
	stricter.html.part \
	repositories.html.part

paludisfaqhtmldir = $(htmldir)/faq

paludisfaqhtml_DATA = \
	index.html \
	different.html \
	general.html \
	howdoi.html \
	misfunctionality.html \
	operation.html \
	stricter.html \
	repositories.html

index.html : header.html.part index.html.part footer.html.part
	cat header.html.part index.html.part footer.html.part > $@

different.html : header.html.part different.html.part footer.html.part
	cat header.html.part different.html.part footer.html.part > $@

general.html : header.html.part general.html.part footer.html.part
	cat header.html.part general.html.part footer.html.part > $@

howdoi.html : header.html.part howdoi.html.part footer.html.part
	cat header.html.part howdoi.html.part footer.html.part > $@

misfunctionality.html : header.html.part misfunctionality.html.part footer.html.part
	cat header.html.part misfunctionality.html.part footer.html.part > $@

operation.html : header.html.part operation.html.part footer.html.part
	cat header.html.part operation.html.part footer.html.part > $@

stricter.html : header.html.part stricter.html.part footer.html.part
	cat header.html.part stricter.html.part footer.html.part > $@

repositories.html : header.html.part repositories.html.part footer.html.part
	cat header.html.part repositories.html.part footer.html.part > $@

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

MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(DIST_SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisfaqhtml_DATA)

