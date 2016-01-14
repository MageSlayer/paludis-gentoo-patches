CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

AUTOMAKE_OPTIONS = 1.11 parallel-tests

noinst_LIBRARIES = liboutput.a
liboutput_a_SOURCES = \
	colour.cc colour.hh \
	colour_pretty_printer.cc colour_pretty_printer.hh

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

