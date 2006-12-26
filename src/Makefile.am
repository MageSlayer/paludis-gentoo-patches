SUBDIRS = . $(BUILD_CLIENTS)
DIST_SUBDIRS = . $(ALL_CLIENTS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = console_install_task-sr.hh console_install_task-sr.cc
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = console_install_task.sr console_install_task-sr.hh console_install_task-sr.cc
BUILT_SOURCES = console_install_task-sr.hh console_install_task-sr.cc

noinst_LIBRARIES = liboutput.a
liboutput_a_SOURCES = \
	colour.cc colour.hh \
	console_install_task.cc console_install_task.hh \
	use_flag_pretty_printer.cc use_flag_pretty_printer.hh \
	licence.cc licence.hh

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

console_install_task-sr.hh : console_install_task.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/console_install_task.sr > $@

console_install_task-sr.cc : console_install_task.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/console_install_task.sr > $@


