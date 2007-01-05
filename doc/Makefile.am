SUBDIRS = examples .

tagfiles = \
	libstdc++.tag \
	libwrapiter.tag

docfiles = \
	doc_main.doxygen \
	doc_mainpage.doxygen \
	doc_coding_standards.doxygen \
	doc_namespaces.doxygen \
	doc_references.doxygen

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
	programmingwithpaludis.html \
	sets.html \
	hooks.html

EXTRA_DIST = doxygen.conf.in header.html footer.html paludis.css \
	$(docfiles) $(tagfiles) htaccess \
	news.html.skel index.html.skel changelog.html.skel licence.html.skel authors.html.skel \
	faq.html.skel htmlheader.html htmlfooter.html migration.html.skel cachefiles.html.skel \
	configuration.html.skel portagedifferences.html.skel programmingwithpaludis.html.skel \
	sets.html.skel hooks.html.skel

CLEANFILES = *~ news.html index.html changelog.html licence.html authors.html faq.html \
	migration.html cachefiles.html configuration.html portagedifferences.html programmingwithpaludis.html \
	sets.html hooks.html.skel \
	cleannews cleanchangelog cleanauthors cleanfaqtoc cleanbasiccppapp cleanbasicrubyapp

MAINTAINERCLEANFILES = Makefile.in $(tagfiles)

libstdc++.tag :
	wget -O $@ http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag

libwrapiter.tag :
	wget -O $@ http://libwrapiter.berlios.de/libwrapiter.tag

www :
	mkdir -p www

all-built-sources :
	$(MAKE) -C .. built-sources

if HAVE_DOXYGEN

doxygen : doxygen.conf $(top_srcdir)/paludis/*.cc $(top_srcdir)/paludis/*.hh \
		$(docfiles) $(tagfiles) www all-built-sources
	mkdir -p www/doxygen/
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

index.html : index.html.skel htmlheader.html htmlfooter.html
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
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

cleanbasiccppapp : $(srcdir)/examples/pwp_basic_cplusplus_app.cc
	sed -e 's,&,\&amp;,g' -e 's,<,\&lt;,g' -e 's,>,\&gt;,g' \
	    < $(srcdir)/examples/pwp_basic_cplusplus_app.cc > cleanbasiccppapp

cleanbasicrubyapp : $(srcdir)/examples/pwp_basic_ruby_app.rb
	sed -e 's,&,\&amp;,g' -e 's,<,\&lt;,g' -e 's,>,\&gt;,g' \
	    < $(srcdir)/examples/pwp_basic_ruby_app.rb > cleanbasicrubyapp

programmingwithpaludis.html : programmingwithpaludis.html.skel htmlheader.html htmlfooter.html \
		cleanbasiccppapp cleanbasicrubyapp
	sed -e '/@HEADER@/r htmlheader.html' \
	    -e '/@HEADER@/d' \
	    -e '/@FOOTER@/r htmlfooter.html' \
	    -e '/@FOOTER@/d' \
	    -e '/@BASIC_CPP_APP@/r cleanbasiccppapp' \
	    -e '/@BASIC_CPP_APP@/d' \
	    -e '/@BASIC_RUBY_APP@/r cleanbasicrubyapp' \
	    -e '/@BASIC_RUBY_APP@/d' \
	    < $(srcdir)/programmingwithpaludis.html.skel > programmingwithpaludis.html

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

clean-local :
	rm -fr www

htmlpages : $(htmlfiles) paludis.css www
	for s in $(htmlfiles) paludis.css ; do cp $(srcdir)/$$s www/ ; done

dothtaccess : htaccess
	cp $(srcdir)/htaccess www/.htaccess

homepage : doxygen rdoc htmlpages dothtaccess

upload-homepage : homepage
	cd `readlink -f $(top_srcdir)/doc/www` && tar jc ./ --exclude ./ | \
		ssh svn.pioto.org tar vjx -C /srv/paludis.pioto.org/www/htdocs/

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

