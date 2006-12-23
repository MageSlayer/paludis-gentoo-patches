MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = _paludis _adjutrix _paludis_packages
EXTRA_DIST = $(noinst_DATA)

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done


