MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = paludis adjutrix qualudis contrarius
EXTRA_DIST = $(noinst_DATA)

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

