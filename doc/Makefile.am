CLEANFILES = *~ html/* html
MAINTAINERCLEANFILES = Makefile.in

docfiles = \
	doc_main.doxygen \
	doc_mainpage.doxygen \
	doc_coding_standards.doxygen \
	doc_namespaces.doxygen \
	doc_directories.doxygen \
	doc_references.doxygen \
	doc_licence.doxygen \
	doc_authors.doxygen

EXTRA_DIST = doxygen.conf.in $(docfiles) HOWTO.rst

if HAVE_DOXYGEN

doxygen : doxygen.conf $(top_srcdir)/paludis/*.cc $(top_srcdir)/paludis/*.hh \
		$(docfiles)
	nice doxygen doxygen.conf

else

doxygen :
	@echo "You don't have doxygen installed!"
	exit 1

endif

HOWTO.html : HOWTO.rst
	rst2html.py $? > $@

upload-howto : HOWTO.html
	scp $? shell.berlios.de:/home/groups/paludis/htdocs/

