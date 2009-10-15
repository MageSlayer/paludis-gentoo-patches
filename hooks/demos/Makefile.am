SUBDIRS = .

DEMOS = \
	report.bash \
	elog.bash \
	new_packages.bash \
	pretend_resume.hook

EXTRA_DIST = \
	report.bash.in \
	elog.bash.in \
	new_packages.bash.in \
	pretend_resume.hook.in

MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.o *.so $(DEMOS) *.epicfail

hooksdemosdir = $(datadir)/paludis/hooks/demos/
hooksdemos_SCRIPTS = $(DEMOS)

new_packages.bash : $(srcdir)/new_packages.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/new_packages.bash.in > new_packages.bash

report.bash : $(srcdir)/report.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/report.bash.in > report.bash

elog.bash : $(srcdir)/elog.bash.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/elog.bash.in > elog.bash

pretend_resume.hook : $(srcdir)/pretend_resume.hook.in
	sed -e "s,@DATADIR@,$(datadir),g" < $(srcdir)/pretend_resume.hook.in > pretend_resume.hook

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

