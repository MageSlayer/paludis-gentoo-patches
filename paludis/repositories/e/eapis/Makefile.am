MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT .keep *.epicfail
SUBDIRS =

AUTOMAKE_OPTIONS = 1.11 parallel-tests

eapis = \
	0.conf \
	1.conf \
	2.conf \
	3.conf \
	4.conf \
	exheres-0.conf \
	paludis-1.conf \
	pbin-1+0.conf \
	pbin-1+1.conf \
	pbin-1+paludis-1.conf \
	pbin-1+exheres-0.conf

future_eapis = \
	4.conf

eapidir = $(datadir)/paludis/eapis/

eapi_DATA = $(eapis)

EXTRA_DIST = $(eapis) $(future_eapis)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

