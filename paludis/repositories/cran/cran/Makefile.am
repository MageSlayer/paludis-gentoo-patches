MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	cran.bash

EXTRA_DIST = $(libexecprog_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then make distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
