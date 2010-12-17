include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = repositories .

CLEANFILES += \
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
	fetchlist \
	specpath.html \
	specpath.html.part \
	general.html \
	general.html.part \
	keywords.html \
	keywords.html.part \
	output.html \
	output.html.part \
	packagemask.html \
	packagemask.html.part \
	suggestions.html \
	suggestions.html.part \
	use.html \
	use.html.part \
	licenses.html \
	licenses.html.part \
	mirrors.html \
	mirrors.html.part \
	bashrc.html \
	bashrc.html.part \
	sets.html \
	sets.html.part \
	configfiles.html \
	specs.html

EXTRA_DIST = \
	toplinks.html.part.in \
	index.html.part \
	hooks.html.part \
	syncers.html.part.in \
	fetchers.html.part.in \
	envvars.html.part \
	specpath.html.part.in \
	general.html.part.in \
	keywords.html.part.in \
	output.html.part.in \
	packagemask.html.part.in \
	suggestions.html.part.in \
	licenses.html.part.in \
	mirrors.html.part.in \
	bashrc.html.part.in \
	use.html.part.in \
	paludisenvironmentonly.html.part \
	configfiles.html.part \
	specs.html.part \
	sets.html.part.in

paludisconfigurationhtmldir = $(htmldir)/configuration

paludisconfigurationhtml_DATA = \
	index.html \
	hooks.html \
	syncers.html \
	fetchers.html \
	envvars.html \
	specpath.html \
	general.html \
	keywords.html \
	packagemask.html \
	suggestions.html \
	mirrors.html \
	output.html \
	bashrc.html \
	licenses.html \
	use.html \
	configfiles.html \
	specs.html \
	sets.html

if HAVE_HTMLTIDY

TESTS = $(paludisconfigurationhtml_DATA)
LOG_COMPILER_INTERP = $(top_srcdir)/doc/htmltidy.bash

endif

index.html : header.html.part $(srcdir)/index.html.part footer.html.part
	cat header.html.part $(srcdir)/index.html.part footer.html.part > $@

specs.html : header.html.part $(srcdir)/specs.html.part footer.html.part
	cat header.html.part $(srcdir)/specs.html.part footer.html.part > $@

configfiles.html : header.html.part $(srcdir)/configfiles.html.part footer.html.part
	cat header.html.part $(srcdir)/configfiles.html.part footer.html.part > $@

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
		elif echo $$a | grep 'svn-base$$' >/dev/null ; then \
			: ; \
		else \
			echo "<strong>$$a</strong>" | sed -e 's/>do/>/' ; \
			if test -f $(top_builddir)/paludis/syncers/$$a ; then \
				env PALUDIS_EBUILD_DIR="$(top_builddir)/paludis/util" \
					bash $(top_builddir)/paludis/syncers/$$a --help || exit 1 ; \
			else \
				env PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/util" \
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
		elif echo $$a | grep 'svn-base$$' >/dev/null ; then \
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

specpath.html.part : $(srcdir)/specpath.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/specpath.html.part.in > specpath.html.part

general.html.part : $(srcdir)/general.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/general.html.part.in > general.html.part

keywords.html.part : $(srcdir)/keywords.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/keywords.html.part.in > keywords.html.part

packagemask.html.part : $(srcdir)/packagemask.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/packagemask.html.part.in > packagemask.html.part

suggestions.html.part : $(srcdir)/suggestions.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/suggestions.html.part.in > suggestions.html.part

mirrors.html.part : $(srcdir)/mirrors.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/mirrors.html.part.in > mirrors.html.part

output.html.part : $(srcdir)/output.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/output.html.part.in > output.html.part

bashrc.html.part : $(srcdir)/bashrc.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/bashrc.html.part.in > bashrc.html.part

sets.html.part : $(srcdir)/sets.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/sets.html.part.in > sets.html.part

licenses.html.part : $(srcdir)/licenses.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/licenses.html.part.in > licenses.html.part

use.html.part : $(srcdir)/use.html.part.in $(srcdir)/paludisenvironmentonly.html.part
	sed \
	    -e '/###PALUDISENVIRONMENTONLY###/r $(srcdir)/paludisenvironmentonly.html.part' \
	    -e 's,###PALUDISENVIRONMENTONLY###,,g' \
	    < $(srcdir)/use.html.part.in > use.html.part

fetchers.html.part : $(srcdir)/fetchers.html.part.in fetchlist
	sed \
	    -e '/###FETCHERS###/r fetchlist' \
	    -e 's,###FETCHERS###,,g' \
	    < $(srcdir)/fetchers.html.part.in > fetchers.html.part

general.html : header.html.part general.html.part footer.html.part
	cat header.html.part general.html.part footer.html.part > $@

keywords.html : header.html.part keywords.html.part footer.html.part
	cat header.html.part keywords.html.part footer.html.part > $@

packagemask.html : header.html.part packagemask.html.part footer.html.part
	cat header.html.part packagemask.html.part footer.html.part > $@

suggestions.html : header.html.part suggestions.html.part footer.html.part
	cat header.html.part suggestions.html.part footer.html.part > $@

mirrors.html : header.html.part mirrors.html.part footer.html.part
	cat header.html.part mirrors.html.part footer.html.part > $@

output.html : header.html.part output.html.part footer.html.part
	cat header.html.part output.html.part footer.html.part > $@

bashrc.html : header.html.part bashrc.html.part footer.html.part
	cat header.html.part bashrc.html.part footer.html.part > $@

sets.html : header.html.part sets.html.part footer.html.part
	cat header.html.part sets.html.part footer.html.part > $@

licenses.html : header.html.part licenses.html.part footer.html.part
	cat header.html.part licenses.html.part footer.html.part > $@

use.html : header.html.part use.html.part footer.html.part
	cat header.html.part use.html.part footer.html.part > $@

specpath.html : header.html.part specpath.html.part footer.html.part
	cat header.html.part specpath.html.part footer.html.part > $@

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

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisconfigurationhtml_DATA)

