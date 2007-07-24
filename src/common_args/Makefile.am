CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

noinst_LIBRARIES = libcommonargs.a

libcommonargs_a_SOURCES = \
	do_help.hh \
	checks_arg.hh      checks_arg.cc \
	debug_build_arg.hh debug_build_arg.cc \
	deps_option_arg.hh deps_option_arg.cc \
	log_level_arg.hh   log_level_arg.cc

