MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = paludis adjutrix qualudis contrarius inquisitio
EXTRA_DIST = $(noinst_DATA)

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

