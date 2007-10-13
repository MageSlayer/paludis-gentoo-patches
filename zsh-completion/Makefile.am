MAINTAINERCLEANFILES = Makefile.in
noinst_DATA = _paludis _adjutrix _paludis_packages _inquisitio
EXTRA_DIST = $(noinst_DATA)

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done


