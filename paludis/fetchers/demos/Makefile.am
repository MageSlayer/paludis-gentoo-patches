SUBDIRS = .

DEMOS = \
	docurl

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.o *.so *.epicfail

fetchersdemosdir = $(datadir)/paludis/fetchers/demos/
fetchersdemos_SCRIPTS = $(DEMOS)

EXTRA_DIST = $(DEMOS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

