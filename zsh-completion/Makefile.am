MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = _paludis _adjutrix _paludis_packages _inquisitio _importare _reconcilio _cave
EXTRA_DIST = $(noinst_DATA)

AUTOMAKE_OPTIONS = 1.11 parallel-tests

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

