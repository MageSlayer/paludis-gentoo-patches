SUBDIRS = examples .

tagfiles = \
	libstdc++.tag \
	libwrapiter.tag

docfiles = \
	doc_main.doxygen \
	doc_mainpage.doxygen \
	doc_coding_standards.doxygen \
	doc_namespaces.doxygen \
	doc_references.doxygen \
	doc_licence.doxygen \
	doc_authors.doxygen \
	doc_bootstrap_howto.doxygen \
	doc_changelog.doxygen \
	doc_portage_differences.doxygen \
	doc_news.doxygen \
	doc_configuration_files.doxygen \
	doc_migration_howto.doxygen \
	doc_known_issues.doxygen \
	doc_programming_with_paludis.doxygen \
	doc_cache_files.doxygen

EXTRA_DIST = doxygen.conf.in header.html footer.html paludis.css $(docfiles) $(tagfiles)
CLEANFILES = *~
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

clean-local :
	rm -fr www

maintainer-clean-local :
	rm -fr www

homepage : doxygen rdoc

upload-homepage : homepage
	cd `readlink -f $(top_srcdir)/doc/www` && tar jc ./ --exclude ./ | \
		ssh shell.berlios.de tar vjx -C /home/groups/paludis/htdocs/

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

