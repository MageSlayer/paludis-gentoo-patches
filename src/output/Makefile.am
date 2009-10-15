CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

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

