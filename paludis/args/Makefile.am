CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

TESTS = \
	args_TEST

paludis_args_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/args/

paludis_args_include_HEADERS = \
	args.hh                \
	args_error.hh          \
	args_group.hh          \
	args_handler.hh        \
	args_option.hh         \
	bad_argument.hh        \
	args_visitor.hh        \
	args_dumper.hh         \
	man.hh                 \
	checks_arg.hh          \
	debug_build_arg.hh     \
	deps_option_arg.hh     \
	log_level_arg.hh       \
	dep_list_args_group.hh \
	install_args_group.hh  \
	do_help.hh

libpaludisargs_la_SOURCES = \
	args.cc \
	args_error.cc \
	args_group.cc \
	args_handler.cc \
	args_option.cc \
	bad_argument.cc \
	args_visitor.cc \
	args_dumper.cc \
	checks_arg.cc \
	debug_build_arg.cc \
	deps_option_arg.cc \
	log_level_arg.cc \
	dep_list_args_group.cc \
	install_args_group.cc

libpaludisman_a_SOURCES = \
	man.cc

libpaludisargs_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisargs_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis.la
	$(top_builddir)/paludis/util/libpaludisutil.la

TESTS_ENVIRONMENT = env PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_DISTRIBUTION="gentoo" \
	bash $(top_srcdir)/test/run_test.sh

lib_LTLIBRARIES = libpaludisargs.la
noinst_LIBRARIES = libpaludisman.a

check_PROGRAMS = $(TESTS)

test_ldadd = \
	libpaludisargs.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil.la

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

DISTCHECK_DEPS = libpaludisargs.la libpaludisman.a

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

