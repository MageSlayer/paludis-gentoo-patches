SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda \
	header.html.part \
	toplinks.html.part \
	footer.html.part

MAINTAINERCLEANFILES = Makefile.in \
	$(ALL_CLIENTS_HTML)

EXTRA_DIST = \
	$(ALL_CLIENTS_HTML) \
	toplinks.html.part.in \
	index.html.part

paludisclientshtmldir = $(htmldir)/clients

paludisclientshtml_DATA = \
	index.html \
	$(ALL_CLIENTS_HTML)

index.html : header.html.part index.html.part footer.html.part
	cat header.html.part index.html.part footer.html.part > $@

paludis.html : header.html.part $(top_builddir)/src/clients/paludis/man-paludis footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/paludis/man-paludis --html && \
	    cat footer.html.part ; } > $@

accerso.html : header.html.part $(top_builddir)/src/clients/accerso/man-accerso footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/accerso/man-accerso --html && \
	    cat footer.html.part ; } > $@

adjutrix.html : header.html.part $(top_builddir)/src/clients/adjutrix/man-adjutrix footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/adjutrix/man-adjutrix --html && \
	    cat footer.html.part ; } > $@

contrarius.html : header.html.part $(top_builddir)/src/clients/contrarius/man-contrarius footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/contrarius/man-contrarius --html && \
	    cat footer.html.part ; } > $@

importare.html : header.html.part $(top_builddir)/src/clients/importare/man-importare footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/importare/man-importare --html && \
	    cat footer.html.part ; } > $@

inquisitio.html : header.html.part $(top_builddir)/src/clients/inquisitio/man-inquisitio footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/inquisitio/man-inquisitio --html && \
	    cat footer.html.part ; } > $@

instruo.html : header.html.part $(top_builddir)/src/clients/instruo/man-instruo footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/instruo/man-instruo --html && \
	    cat footer.html.part ; } > $@

qualudis.html : header.html.part $(top_builddir)/src/clients/qualudis/man-qualudis footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/qualudis/man-qualudis --html && \
	    cat footer.html.part ; } > $@

reconcilio.html : header.html.part $(top_builddir)/src/clients/reconcilio/man-reconcilio footer.html.part
	{ cat header.html.part && \
	    $(top_builddir)/src/clients/reconcilio/man-reconcilio --html && \
	    cat footer.html.part ; } > $@

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

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(DIST_SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisclientshtml_DATA)

