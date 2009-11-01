MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ *.epicfail
SUBDIRS = .

AUTOMAKE_OPTIONS = parallel-tests

libexecprogdir = $(libexecdir)/paludis/

libexecprog_SCRIPTS = \
	cran.bash

EXTRA_DIST = $(libexecprog_SCRIPTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

