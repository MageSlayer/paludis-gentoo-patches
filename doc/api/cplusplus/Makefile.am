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

doxygen_files = \
	groups.doxygen \
	main_page.doxygen \
	namespaces.doxygen \
	references.doxygen

EXTRA_DIST = doxygen.conf.in $(doxygen_files)
CLEANFILES = *~ doxygen/* paludis.tag

MAINTAINERCLEANFILES = Makefile.in doxygen.conf $(tagfiles)

if HAVE_DOXYGEN

doxygen : doxygen.conf $(doxygen_files) $(tagfiles)
	doxygen doxygen.conf

else

doxygen :
	@echo "You don't have doxygen installed!"
	exit 1

endif

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

clean-local :
	rm -fr doxygen/

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludishtmlapicplusplus_DATA)

