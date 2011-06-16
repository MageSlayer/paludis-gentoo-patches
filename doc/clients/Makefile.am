include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

CLEANFILES += \
	header.html.part \
	toplinks.html.part \
	footer.html.part \
	index.html

DISTCLEANFILES = \
	$(ALL_CLIENTS_HTML) \
	$(CAVE_COMMANDS_HTML)

EXTRA_DIST = \
	$(ALL_CLIENTS_HTML) \
	$(CAVE_COMMANDS_HTML) \
	toplinks.html.part.in \
	index.html.part

paludisclientshtmldir = $(htmldir)/clients

paludisclientshtml_DATA = \
	index.html \
	$(ALL_CLIENTS_HTML) \
	$(CAVE_COMMANDS_HTML)

CAVE_COMMANDS_HTML = \
	cave-config.html \
	cave-contents.html \
	cave-digest.html \
	cave-display-resolution.html \
	cave-dump-cave-formats-conf.html \
	cave-executables.html \
	cave-execute-resolution.html \
	cave-find-candidates.html \
	cave-fix-cache.html \
	cave-fix-linkage.html \
	cave-generate-metadata.html \
	cave-graph-jobs.html \
	cave-has-version.html \
	cave-help.html \
	cave-import.html \
	cave-info.html \
	cave-manage-search-index.html \
	cave-match.html \
	cave-mirror.html \
	cave-owner.html \
	cave-perform.html \
	cave-print-best-version.html \
	cave-print-categories.html \
	cave-print-commands.html \
	cave-print-dependent-ids.html \
	cave-print-environment-metadata.html \
	cave-print-id-actions.html \
	cave-print-id-contents.html \
	cave-print-id-environment-variable.html \
	cave-print-id-executables.html \
	cave-print-id-masks.html \
	cave-print-id-metadata.html \
	cave-print-id-size.html \
	cave-print-ids.html \
	cave-print-owners.html \
	cave-print-packages.html \
	cave-print-repositories.html \
	cave-print-repository-formats.html \
	cave-print-repository-metadata.html \
	cave-print-set.html \
	cave-print-sets.html \
	cave-print-spec.html \
	cave-print-sync-protocols.html \
	cave-print-unused-distfiles.html \
	cave-purge.html \
	cave-report.html \
	cave-resolve.html \
	cave-resume.html \
	cave-search.html \
	cave-show.html \
	cave-size.html \
	cave-sync.html \
	cave-sync-protocol-options.html \
	cave-uninstall.html \
	cave-update-world.html \
	cave-verify.html

if HAVE_HTMLTIDY

TESTS = $(paludisclientshtml_DATA)
LOG_COMPILER_INTERP = $(top_srcdir)/doc/htmltidy.bash

endif

# automake is retartded
TEST_EXTENSIONS = .html
HTML_LOG_COMPILER = $(LOG_COMPILER)

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

cave.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    echo "<h1>cave</h1>"; \
	    if test -f $(top_builddir)/src/clients/cave/cave.html-man-fragment ; then \
	        cat $(top_builddir)/src/clients/cave/cave.html-man-fragment ; \
	    elif test -f $(top_srcdir)/src/clients/cave/cave.html-man-fragment ; then \
	        cat $(top_srcdir)/src/clients/cave/cave.html-man-fragment ; \
	    else \
	        echo "<p>Sorry, documentation was generated without support for the cave client.</p>" ; \
	    fi ; } | sed \
	        -e '/<h2>SEE ALSO<.h2>/,/<.div>/s#<span class="strong"><strong>\(.*\)</strong></span>(1)#<a href="\1.html">\1</a>#' \
	        -e '/<h2>CORE COMMANDS<.h2>/,/<h2>ENVIRONMENT<.h2>/s,^\([a-z-]\+\)$$,<a href="cave-\1.html">\1</a>,' \
	    && cat footer.html.part ; } > $@

$(CAVE_COMMANDS_HTML) : header.html.part footer.html.part
	{ cat header.html.part && { \
	    echo "<h1>cave `basename $@ .html | sed -e 's,cave-,,'`</h1>"; \
	    if test -f $(top_builddir)/src/clients/cave/`basename $@ .html `.html-man-fragment ; then \
	        cat $(top_builddir)/src/clients/cave/`basename $@ .html `.html-man-fragment ; \
	    elif test -f $(top_srcdir)/src/clients/cave/`basename $@ .html `.html-man-fragment ; then \
	        cat $(top_srcdir)/src/clients/cave/`basename $@ .html `.html-man-fragment ; \
	    else \
	        echo "<p>Sorry, documentation was generated without support for the cave client.</p>" ; \
	    fi ; } \
	    && cat footer.html.part ; } > $@

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

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisclientshtml_DATA)

