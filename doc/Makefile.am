CLEANFILES = *~ html/* html
MAINTAINERCLEANFILES = Makefile.in

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
	doc_security_advisories.doxygen \
	doc_configuration_files.doxygen \
	doc_migration_howto.doxygen \
	doc_known_issues.doxygen

EXTRA_DIST = doxygen.conf.in header.html footer.html paludis.css $(docfiles)

if HAVE_DOXYGEN

doxygen : doxygen.conf $(top_srcdir)/paludis/*.cc $(top_srcdir)/paludis/*.hh \
		$(docfiles)
	doxygen doxygen.conf

else

doxygen :
	@echo "You don't have doxygen installed!"
	exit 1

endif

clean-local :
	mkdir -p html
	touch html/dummy
	find html/ -type f | grep -v '\.svn' | xargs rm

maintainer-clean-local :
	mkdir -p html
	touch html/dummy
	find html/ -type f | grep -v '\.svn' | xargs rm

upload-homepage : doxygen
	cd `readlink -f $(top_srcdir)/doc/html` && tar jc ./ | \
		ssh shell.berlios.de tar vjx -C /home/groups/paludis/htdocs/

