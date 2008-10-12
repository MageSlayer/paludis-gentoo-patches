CLEANFILES = *~ *.epicfail
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = generated-file.txt do_m4.bash make_sr.bash make_se.bash make_nn.bash

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
