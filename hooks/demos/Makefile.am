SUBDIRS = .

DEMOS = \
	report.bash \
	elog.bash

EXTRA_DIST = \
	report.bash.in \
	elog.bash.in

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda *.o *.so $(DEMOS)

hooksdemosdir = $(datadir)/paludis/hooks/demos/
hooksdemos_DATA = $(DEMOS)

report.bash : $(srcdir)/report.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/report.bash.in > report.bash

elog.bash : $(srcdir)/elog.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/elog.bash.in > elog.bash

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

