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
	cave-display-resolution.html \
	cave-dump-cave-formats-conf.html \
	cave-executables.html \
	cave-execute-resolution.html \
	cave-find-candidates.html \
	cave-fix-cache.html \
	cave-fix-linkage.html \
	cave-help.html \
	cave-import.html \
	cave-info.html \
	cave-manage-search-index.html \
	cave-match.html \
	cave-owner.html \
	cave-perform.html \
	cave-print-categories.html \
	cave-print-commands.html \
	cave-print-environment-metadata.html \
	cave-print-id-actions.html \
	cave-print-id-contents.html \
	cave-print-id-environment-variable.html \
	cave-print-id-executables.html \
	cave-print-id-masks.html \
	cave-print-id-metadata.html \
	cave-print-ids.html \
	cave-print-owners.html \
	cave-print-packages.html \
	cave-print-repositories.html \
	cave-print-repository-formats.html \
	cave-print-repository-metadata.html \
	cave-print-set.html \
	cave-print-sets.html \
	cave-print-sync-protocols.html \
	cave-purge.html \
	cave-report.html \
	cave-resolve.html \
	cave-resume.html \
	cave-search.html \
	cave-show.html \
	cave-sync.html \
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

paludis.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/paludis/man-paludis ; then \
	        $(top_builddir)/src/clients/paludis/man-paludis --html ; \
	    else \
	        echo "<h1>paludis</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the paludis client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

accerso.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/accerso/man-accerso ; then \
	        $(top_builddir)/src/clients/accerso/man-accerso --html ; \
	    else \
	        echo "<h1>accerso</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the accerso client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

adjutrix.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/adjutrix/man-adjutrix ; then \
	        $(top_builddir)/src/clients/adjutrix/man-adjutrix --html ; \
	    else \
	        echo "<h1>adjutrix</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the adjutrix client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

appareo.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/appareo/man-appareo ; then \
	        $(top_builddir)/src/clients/appareo/man-appareo --html ; \
	    else \
	        echo "<h1>appareo</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the appareo client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

cave.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/cave/man-cave ; then \
	        env CAVE_COMMANDS_PATH="" $(top_builddir)/src/clients/cave/man-cave --html | \
	        sed -e '/<h2>See Also<.h2>/,/<.ul>/s#<strong>\(.*\)</strong> (1)#<a href="\1.html">\1</a>#'; \
	    else \
	        echo "<h1>cave</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the cave client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

$(CAVE_COMMANDS_HTML) : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/cave/man-cave ; then \
	        basename $@ .html | sed -e 's,^cave-,,' | xargs env CAVE_COMMANDS_PATH="" $(top_builddir)/src/clients/cave/man-cave --html | \
	        sed -e '/<h2>See Also<.h2>/,/<.ul>/s#<strong>\(.*\)</strong> (1)#<a href="\1.html">\1</a>#'; \
	    else \
	        echo "<h1>cave</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the cave client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

inquisitio.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/inquisitio/man-inquisitio ; then \
	        $(top_builddir)/src/clients/inquisitio/man-inquisitio --html ; \
	    else \
	        echo "<h1>inquisitio</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the inquisitio client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

importare.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/importare/man-importare ; then \
	        $(top_builddir)/src/clients/importare/man-importare --html ; \
	    else \
	        echo "<h1>importare</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the importare client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

instruo.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/instruo/man-instruo ; then \
	        $(top_builddir)/src/clients/instruo/man-instruo --html ; \
	    else \
	        echo "<h1>instruo</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the instruo client.</p>" ; \
	    fi ; } && \
	    cat footer.html.part ; } > $@

reconcilio.html : header.html.part footer.html.part
	{ cat header.html.part && { \
	    if test -x $(top_builddir)/src/clients/reconcilio/man-reconcilio ; then \
	        $(top_builddir)/src/clients/reconcilio/man-reconcilio --html ; \
	    else \
	        echo "<h1>reconcilio</h1>"; \
	        echo "<p>Sorry, documentation was generated without support for the reconcilio client.</p>" ; \
	    fi ; } && \
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

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisclientshtml_DATA)

