MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda .keep
SUBDIRS =

distributions = \
	gentoo.conf

distributionsdir = $(datadir)/paludis/distributions/

distributions_DATA = $(distributions)

EXTRA_DIST = $(distributions)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
