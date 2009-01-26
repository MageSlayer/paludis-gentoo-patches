SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html \
	accounts.html \
	e.html \
	exndbam.html \
	installed_accounts.html \
	installed_unpackaged.html \
	unavailable.html \
	vdb.html

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html.part \
	accounts.html \
	e.html \
	exndbam.html \
	installed_accounts.html \
	installed_unpackaged.html \
	unavailable.html \
	vdb.html

paludisconfigurationrepositorieshtmldir = $(htmldir)/configuration/repositories/

paludisconfigurationrepositorieshtml_DATA = \
	index.html \
	accounts.html \
	e.html \
	exndbam.html \
	installed_accounts.html \
	installed_unpackaged.html \
	unavailable.html \
	vdb.html

if HAVE_HTMLTIDY

TESTS = $(paludisconfigurationhtml_DATA)
TESTS_ENVIRONMENT = bash $(top_srcdir)/doc/htmltidy.bash

endif

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

accounts.html : header.html.part $(srcdir)/accounts.html.part footer.html.part
	cat header.html.part $(srcdir)/accounts.html.part footer.html.part > $@

e.html : header.html.part $(srcdir)/e.html.part footer.html.part
	cat header.html.part $(srcdir)/e.html.part footer.html.part > $@

exndbam.html : header.html.part $(srcdir)/exndbam.html.part footer.html.part
	cat header.html.part $(srcdir)/exndbam.html.part footer.html.part > $@

installed_accounts.html : header.html.part $(srcdir)/installed_accounts.html.part footer.html.part
	cat header.html.part $(srcdir)/installed_accounts.html.part footer.html.part > $@

installed_unpackaged.html : header.html.part $(srcdir)/installed_unpackaged.html.part footer.html.part
	cat header.html.part $(srcdir)/installed_unpackaged.html.part footer.html.part > $@

unavailable.html : header.html.part $(srcdir)/unavailable.html.part footer.html.part
	cat header.html.part $(srcdir)/unavailable.html.part footer.html.part > $@

vdb.html : header.html.part $(srcdir)/vdb.html.part footer.html.part
	cat header.html.part $(srcdir)/vdb.html.part footer.html.part > $@

header.html.part : $(srcdir)/../../header.html.part.in toplinks.html.part
	sed \
	    -e 's,###TOPURI###,../../,g' \
	    -e '/###TOPLINKS###/r $(srcdir)/toplinks.html.part' \
	    -e 's,###TOPLINKS###,,g' \
	    < $(srcdir)/../../header.html.part.in > header.html.part

toplinks.html.part : $(srcdir)/toplinks.html.part.in
	sed \
	    -e 's,###TOPURI###,../../,g' \
	    < $(srcdir)/toplinks.html.part.in > toplinks.html.part

footer.html.part : $(srcdir)/../../footer.html.part.in
	sed -e 's,###TOPURI###,../../,g' < $(srcdir)/../../footer.html.part.in > footer.html.part

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(DIST_SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisconfigurationrepositorieshtml_DATA)

