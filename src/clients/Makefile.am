SUBDIRS = . $(BUILD_CLIENTS)
DIST_SUBDIRS = . $(ALL_CLIENTS)

AUTOMAKE_OPTIONS = 1.11 parallel-tests

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

