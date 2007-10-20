CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = console_install_task-sr.hh console_install_task-sr.cc
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = console_install_task.sr console_install_task-sr.hh console_install_task-sr.cc
BUILT_SOURCES = console_install_task-sr.hh console_install_task-sr.cc

noinst_LIBRARIES = liboutput.a
liboutput_a_SOURCES = \
	colour.cc colour.hh \
	colour_formatter.cc colour_formatter.hh \
	console_task.cc console_task.hh \
	console_install_task.cc console_install_task.hh \
	console_query_task.cc console_query_task.hh \
	mask_displayer.cc mask_displayer.hh

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

DISTCHECK_DEPS = liboutput.a

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

console_install_task-sr.hh : console_install_task.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/console_install_task.sr > $@

console_install_task-sr.cc : console_install_task.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/console_install_task.sr > $@


