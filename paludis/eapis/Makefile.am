MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS =

eapis = \
	0.conf \
	CRAN-1.conf \
	exheres-0.conf \
	paludis-1.conf

eapidir = $(datadir)/paludis/eapis/

eapi_DATA = $(eapis)

EXTRA_DIST = $(eapis)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

