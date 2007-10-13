SUBDIRS = examples .

if HAVE_DOXYGEN_TAGS

tagfiles = \
	libstdc++.tag \
	libwrapiter.tag

libstdc++.tag :
	wget -O $@ http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag

libwrapiter.tag :
	wget -O $@ http://libwrapiter.pioto.org/libwrapiter.tag

endif

docfiles = \
	doc_main.doxygen \
	doc_mainpage.doxygen \
	doc_coding_standards.doxygen \
	doc_namespaces.doxygen \
	doc_references.doxygen

images = \
	 paludis.svg \
	 paludis_270.png

htmlfiles = \
	index.html \
	news.html \
	changelog.html \
	licence.html \
	authors.html \
	faq.html \
	migration.html \
	cachefiles.html \
	configuration.html \
	portagedifferences.html \
	sets.html \
	hooks.html \
	man-paludis.html \
	man-qualudis.html \
	man-contrarius.html \
	man-adjutrix.html \
	man-accerso.html \
	man-instruo.html \
	man-inquisitio.html

EXTRA_DIST = doxygen.conf.in header.html footer.html paludis.css epydoc.css \
	$(docfiles) $(images) htaccess \
	news.html.skel index.html.skel changelog.html.skel licence.html.skel authors.html.skel \
	faq.html.skel htmlheader.html htmlfooter.html migration.html.skel cachefiles.html.skel \
	configuration.html.skel portagedifferences.html.skel \
	sets.html.skel hooks.html.skel man.html.skel

CLEANFILES = *~ news.html index.html changelog.html licence.html authors.html faq.html \
	migration.html cachefiles.html configuration.html portagedifferences.html \
	sets.html hooks.html man-paludis.html man-qualudis.html man-contrarius.html man-adjutrix.html man-inquisitio.html \
	man-accerso.html man-instruo.html \
	cleannews cleanrecentnews cleanchangelog cleanauthors cleanfaqtoc cleanbasiccppapp cleanbasicrubyapp

MAINTAINERCLEANFILES = Makefile.in $(tagfiles)

www :
	mkdir -p www

all-built-sources :
	$(MAKE) -C .. built-sources

if HAVE_DOXYGEN

doxygen : doxygen.conf $(top_srcdir)/paludis/*.cc $(top_srcdir)/paludis/*.hh \
		$(docfiles) $(tagfiles) www all-built-sources
	mkdir -p www/doxygen/html
	cp paludis_270.png www/doxygen/html
	doxygen doxygen.conf

else

doxygen :
	@echo "You don't have doxygen installed!"
	exit 1

endif

if ENABLE_RUBY

rdoc : create_ruby_doc.rb www all-built-sources
	$(RUBY) $(srcdir)/create_ruby_doc.rb -t "Paludis Ruby API" -m Paludis --op www/ruby/ $(top_srcdir)/ruby/*.cc

else

rdoc :
	@echo "You don't have ruby turned on!"
	exit 1

endif

if ENABLE_PYTHON

epydoc : epydoc.css www all-built-sources
	mkdir -p www/python/doc
	epydoc -n Paludis -o www/python/doc -u http://paludis.pioto.org --no-frames -c epydoc.css \
		$(top_builddir)/python/paludis.so

else

epydoc :
	@echo "You don't have python turned on!"
	exit 1

endif


cleanauthors : $(top_srcdir)/AUTHORS
	sed -e '1,3d' \
	    -e 's,&,\&amp;,g' -e 's,<,\&lt;,g' -e 's,>,\&gt;,g' -e 's,@, at ,g' \
	    -e 's,^    \(.*\),<dd>\1</dd>,' \
	    -e 's,^\([^ <].*\),<dt>\1</dt>,' \
	    < $(top_srcdir)/AUTHORS > cleanauthors

authors.html : authors.html.skel cleanauthors htmlheader.html htmlfooter.html
	sed -e '/@AUTHORS@/r cleanauthors' -e '/@AUTHORS@/d' \
	    -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' < $(srcdir)/authors.html.skel > authors.html

cleannews : $(top_srcdir)/NEWS
	sed -e '1,6d' -e '$$d' < $(top_srcdir)/NEWS > cleannews

news.html : news.html.skel cleannews htmlheader.html htmlfooter.html
	sed -e '/@RELEASE_NOTES@/r cleannews' -e '/@RELEASE_NOTES@/d' \
	    -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' < $(srcdir)/news.html.skel | \
	    sed \
	        -e 's~^\([0-9].*\):~</li></ul><h3>&</h3><ul>~' \
	        -e 's~^    \*~</li><li>~' \
	    | tr '\012' '\007' \
	    | sed \
	        -e 's~</li></ul><h3>~<h3>~' \
	        -e 's~<ul>\a*</li>~<ul>~g' \
	    | tr '\007' '\012' > news.html

cleanchangelog : $(top_srcdir)/ChangeLog
	sed -e '1,7d' -e '$$d' -e 's,&,\&amp;,g' -e 's,<,\&lt;,g' -e 's,>,\&gt;,g' -e 's,@, at ,g' \
	    < $(top_srcdir)/ChangeLog \
	    | sed \
	        -e 's~^\(200[0-9]-[0-9][0-9].*\)~<dt>&</dt>~' \
	    | tr '\012' '\007' \
	    | sed -e 's~\a\t[\*\+] ~\a<dd>~g' \
	    | sed -e 's~\([^>]\)\a\a~\1</dd>\a~g' \
	    | sed \
	        -e 's,<dd>\([^:<]\+\):,<dd><code>\1</code>:<br />,g' \
	    | tr '\007' '\012' > cleanchangelog

changelog.html : changelog.html.skel cleanchangelog htmlheader.html htmlfooter.html
	sed -e '/@CHANGELOG@/r cleanchangelog' -e '/@CHANGELOG@/d' \
	    -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' < $(srcdir)/changelog.html.skel > changelog.html

cleanrecentnews : news.html
	sed -n -e '/^<h3>[0-9]\+\.[^:]\+:/,/<.li><.ul><h3>/p' < news.html \
	    | sed -e '1s/.*<.h3>//' -e '$$s,.*,</ul>,' > cleanrecentnews

index.html : index.html.skel htmlheader.html htmlfooter.html cleanrecentnews
	sed -e '/[@]HEADER@/r htmlheader.html' \
	    -e '/[@]HEADER@/d' \
	    -e '/[@]FOOTER@/r htmlfooter.html' \
	    -e '/[@]FOOTER@/d' \
	    -e 's/[@]VERSION@/$(VERSION_FULL)/g' \
	    -e '/[@]RECENTNEWS@/r cleanrecentnews' \
	    -e '/[@]RECENTNEWS@/d' \
	    < $(srcdir)/index.html.skel > index.html

configuration.html : configuration.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/configuration.html.skel > configuration.html

sets.html : sets.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/sets.html.skel > sets.html

hooks.html : hooks.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/hooks.html.skel > hooks.html

migration.html : migration.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/migration.html.skel > migration.html

portagedifferences.html : portagedifferences.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/portagedifferences.html.skel > portagedifferences.html

cachefiles.html : cachefiles.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    < $(srcdir)/cachefiles.html.skel > cachefiles.html

faq.html : faq.html.skel htmlheader.html htmlfooter.html cleanfaqtoc
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@TOC@/r cleanfaqtoc' \
	    -e '/@TOC@/d' \
	    < $(srcdir)/faq.html.skel > faq.html

cleanfaqtoc : faq.html.skel
	sed -n -e 's!^<h\([23]\) id="\([^"]\+\)">\(.*\)</h\1>!\1 <li><a href="#\2">\3</a></li>!gp' \
	    < $(srcdir)/faq.html.skel \
	    | sed -e 's!^2 <li>\(.*\)</li>!</ul></li><li><strong>\1</strong><ul>!' \
	        -e 's!^3 !!' \
	    | sed -e '1s!^</ul></li>!<ul>!' \
	        -e '$$s!$$!</ul></li></ul>!' \
	    > cleanfaqtoc

licence.html : licence.html.skel htmlheader.html htmlfooter.html ../COPYING
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@COPYING@/r ../COPYING' \
	    -e '/@COPYING@/d' \
	    < $(srcdir)/licence.html.skel > licence.html

man-adjutrix.html : $(top_builddir)/src/clients/adjutrix/adjutrix.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/adjutrix/adjutrix.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-adjutrix.html

man-accerso.html : $(top_builddir)/src/clients/accerso/accerso.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/accerso/accerso.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-accerso.html

man-instruo.html : $(top_builddir)/src/clients/instruo/instruo.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/instruo/instruo.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-instruo.html

man-inquisitio.html : $(top_builddir)/src/clients/inquisitio/inquisitio.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/inquisitio/inquisitio.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-inquisitio.html

man-qualudis.html : $(top_builddir)/src/clients/qualudis/qualudis.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/qualudis/qualudis.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-qualudis.html

man-paludis.html : $(top_builddir)/src/clients/paludis/paludis.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/paludis/paludis.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-paludis.html

man-contrarius.html : $(top_builddir)/src/clients/contrarius/contrarius.html man.html.skel
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@MAN@/r $(top_builddir)/src/clients/contrarius/contrarius.html' \
	    -e '/@MAN@/d' \
	    < $(srcdir)/man.html.skel > man-contrarius.html

clean-local :
	rm -fr www

htmlpages : $(htmlfiles) paludis.css www
	for s in $(htmlfiles) paludis.css ; do cp $(srcdir)/$$s www/ ; done

dothtaccess : htaccess
	cp $(srcdir)/htaccess www/.htaccess

homepage : doxygen rdoc epydoc htmlpages dothtaccess images

images : $(images)
	for s in $(images) ; do cp $(srcdir)/$$s www/ ; done

upload-homepage : homepage upload-homepage-real

upload-homepage-real :
	cd `readlink -f $(top_srcdir)/doc/www` && tar jc ./ --exclude ./ | \
		ssh svn.pioto.org tar vjx -C /srv/paludis.pioto.org/www/htdocs/

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done


