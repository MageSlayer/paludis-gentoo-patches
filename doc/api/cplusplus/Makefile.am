SUBDIRS = examples .

if HAVE_DOXYGEN_TAGS

tagfiles = \
	libstdc++.tag

libstdc++.tag :
	wget -O $@ http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag

endif

doxygen_files = \
	groups.doxygen \
	main_page.doxygen \
	namespaces.doxygen \
	references.doxygen

EXTRA_DIST = doxygen.conf.in $(doxygen_files)
CLEANFILES = *~ cplusplus/* cplusplus paludis.tag *.epicfail

MAINTAINERCLEANFILES = Makefile.in doxygen.conf $(tagfiles)

if HAVE_DOXYGEN

all-local : cplusplus

doxygen : cplusplus

cplusplus : doxygen.conf $(doxygen_files) $(tagfiles)
	doxygen doxygen.conf

install-data-local :
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R cplusplus/ $(DESTDIR)$(htmldir)/api/

else

doxygen :
	@echo "You don't have doxygen installed!"
	exit 1

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

clean-local :
	rm -fr cplusplus/

uninstall-local :
	rm -fr $(DESTDIR)$(htmldir)/api/cplusplus

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludishtmlapicplusplus_DATA)

