MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = _paludis _adjutrix _paludis_packages _inquisitio _importare _reconcilio
EXTRA_DIST = $(noinst_DATA)

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

