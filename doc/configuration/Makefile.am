SUBDIRS = .

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html \
	hooks.html \
	syncers.html.part \
	syncers.html \
	fetchers.html.part \
	fetchers.html \
	envvars.html \
	synclist \
	fetchlist

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	toplinks.html.part.in \
	index.html.part \
	hooks.html.part \
	syncers.html.part.in \
	fetchers.html.part.in \
	envvars.html.part

paludisconfigurationhtmldir = $(htmldir)/configuration

paludisconfigurationhtml_DATA = \
	index.html \
	hooks.html \
	syncers.html \
	fetchers.html \
	envvars.html

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

envvars.html : header.html.part $(srcdir)/envvars.html.part footer.html.part
	cat header.html.part $(srcdir)/envvars.html.part footer.html.part > $@

synclist :
	find $(top_srcdir)/paludis/syncers $(top_builddir)/paludis/syncers -type f \
	    | while read a ; do \
		basename $$a ; \
	done | sort -u | \
	while read a ; do \
		if echo $$a | grep '.in$$' >/dev/null ; then \
			: ; \
		elif ! echo $$a | grep '^do' >/dev/null ; then \
			: ; \
		else \
			echo "<strong>$$a</strong>" | sed -e 's/>do/>/' ; \
			if test -f $(top_builddir)/paludis/syncers/$$a ; then \
				env PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild" \
					bash $(top_builddir)/paludis/syncers/$$a --help || exit 1 ; \
			else \
				env PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/e/ebuild" \
					bash $(top_srcdir)/paludis/syncers/$$a --help || exit 1 ; \
			fi ; \
			echo ; \
		fi ; \
	done > synclist

fetchlist :
	find $(top_srcdir)/paludis/fetchers $(top_builddir)/paludis/fetchers -type f \
	    | while read a ; do \
		basename $$a ; \
	done | sort -u | \
	while read a ; do \
		if echo $$a | grep '.in$$' >/dev/null ; then \
			: ; \
		elif ! echo $$a | grep '^do' >/dev/null ; then \
			: ; \
		else \
			echo "<li>$$a</li>" | sed -e 's/>do/>/' ; \
		fi ; \
	done > fetchlist

syncers.html.part : $(srcdir)/syncers.html.part.in synclist
	sed \
	    -e '/###SYNCERS###/r synclist' \
	    -e 's,###SYNCERS###,,g' \
	    < $(srcdir)/syncers.html.part.in > syncers.html.part

fetchers.html.part : $(srcdir)/fetchers.html.part.in fetchlist
	sed \
	    -e '/###FETCHERS###/r fetchlist' \
	    -e 's,###FETCHERS###,,g' \
	    < $(srcdir)/fetchers.html.part.in > fetchers.html.part

syncers.html : header.html.part syncers.html.part footer.html.part
	cat header.html.part syncers.html.part footer.html.part > $@

fetchers.html : header.html.part fetchers.html.part footer.html.part
	cat header.html.part fetchers.html.part footer.html.part > $@

hooks.html : header.html.part $(srcdir)/hooks.html.part footer.html.part
	cat header.html.part $(srcdir)/hooks.html.part footer.html.part > $@

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

htmldocs-local : $(paludisconfigurationhtml_DATA)

