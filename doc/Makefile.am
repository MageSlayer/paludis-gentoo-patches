SUBDIRS = . api clients configuration faq overview

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ *.epicfail \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html.part \
	news.html.part \
	news.subst \
	changelog.html.part \
	changelog.subst \
	index.html \
	news.html \
	changelog.html

EXTRA_DIST = \
	header.html.part.in \
	toplinks.html.part.in \
	footer.html.part.in \
	index.html.part.in \
	news.html.part.in \
	changelog.html.part.in \
	paludis.css \
	paludis_270.png \
	paludis_64.png

paludishtmldir = $(htmldir)

paludishtml_DATA = \
	index.html \
	news.html \
	changelog.html \
	paludis.css \
	paludis_270.png

dist_check_SCRIPTS = \
	htmltidy.bash

if HAVE_HTMLTIDY

TESTS = $(paludishtml_DATA)
TESTS_ENVIRONMENT = bash $(top_srcdir)/doc/htmltidy.bash

endif

index.html : header.html.part index.html.part footer.html.part
	cat header.html.part index.html.part footer.html.part > $@

news.html : header.html.part news.html.part footer.html.part
	cat header.html.part news.html.part footer.html.part > $@

changelog.html : header.html.part changelog.html.part footer.html.part
	cat header.html.part changelog.html.part footer.html.part > $@

header.html.part : $(srcdir)/header.html.part.in toplinks.html.part
	sed \
	    -e 's,###TOPURI###,./,g' \
	    -e '/###TOPLINKS###/r $(srcdir)/toplinks.html.part' \
	    -e 's,###TOPLINKS###,,g' \
	    < $(srcdir)/header.html.part.in > header.html.part

toplinks.html.part : $(srcdir)/toplinks.html.part.in
	sed \
	    -e 's,###TOPURI###,./,g' \
	    < $(srcdir)/toplinks.html.part.in > toplinks.html.part

footer.html.part : $(srcdir)/footer.html.part.in
	sed -e 's,###TOPURI###,./,g' < $(srcdir)/footer.html.part.in > footer.html.part

index.html.part : $(srcdir)/index.html.part.in
	sed -e 's,###PALUDIS_VERSION###,$(VERSION_FULL),g' < $(srcdir)/index.html.part.in > index.html.part

news.html.part : $(srcdir)/news.html.part.in news.subst
	sed \
	    -e '/###NEWS###/r news.subst' \
	    -e 's,###NEWS###,,' \
	    < $(srcdir)/news.html.part.in > news.html.part

changelog.html.part : $(srcdir)/changelog.html.part.in changelog.subst
	sed \
	    -e '/###CHANGELOG###/r changelog.subst' \
	    -e 's,###CHANGELOG###,,' \
	    < $(srcdir)/changelog.html.part.in > changelog.html.part

news.subst : $(top_srcdir)/NEWS
	sed \
	    -e 's,&,\&amp;,g' \
	    -e 's,<,\&lt;,g' \
	    -e 's,>,\&gt;,g' \
	    < $(top_srcdir)/NEWS > news.subst

changelog.subst :
	git log | sed \
	    -e 's,&,\&amp;,g' \
	    -e 's,<,\&lt;,g' \
	    -e 's,>,\&gt;,g' \
	    -e 's,\([[:blank:][:punct:]]\)\?\([0-9a-f]\{40\}\)\([[:blank:][:punct:]]\)\?,\1<a href="http://git.pioto.org/?p=paludis.git;a=commit;h=\2">\2</a>,g' \
	    -e 's,ticket:\([0-9]\+\),<a href="http://trac.pioto.org/paludis/ticket/\1">ticket:\1</a>,g' \
	    > changelog.subst

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

documentation : doxygen epydoc htmldocs

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludishtml_DATA)

doxygen :
	$(MAKE) -C api $@

epydoc :
	$(MAKE) -C api $@

