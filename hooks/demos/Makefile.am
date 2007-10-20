SUBDIRS = .

DEMOS = \
	report.bash \
	elog.bash \
	new_packages.bash

EXTRA_DIST = \
	report.bash.in \
	elog.bash.in \
	new_packages.bash.in

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o *.so $(DEMOS)

hooksdemosdir = $(datadir)/paludis/hooks/demos/
hooksdemos_SCRIPTS = $(DEMOS)

new_packages.bash : $(srcdir)/new_packages.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/new_packages.bash.in > new_packages.bash

report.bash : $(srcdir)/report.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/report.bash.in > report.bash

elog.bash : $(srcdir)/elog.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/elog.bash.in > elog.bash

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

