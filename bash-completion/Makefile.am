MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = paludis adjutrix \
	      inquisitio \
	      accerso instruo reconcilio \
	      importare
EXTRA_DIST = $(noinst_DATA)

AUTOMAKE_OPTIONS = parallel-tests

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

