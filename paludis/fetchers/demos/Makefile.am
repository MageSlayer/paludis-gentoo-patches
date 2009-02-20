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

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

