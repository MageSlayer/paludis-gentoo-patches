SUBDIRS = . paludis gtkpaludis qualudis adjutrix

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = liboutput.a
liboutput_a_SOURCES = \
	colour.cc colour.hh \
	use.cc use.hh \
	console_install_task.cc console_install_task.hh

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

