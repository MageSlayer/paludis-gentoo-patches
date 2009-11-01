CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = args_handler-se.hh args_handler-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = args_handler-se.hh args_handler-se.cc args_handler.se
BUILT_SOURCES = args_handler-se.hh args_handler-se.cc

AUTOMAKE_OPTIONS = parallel-tests

TESTS = \
	args_TEST

paludis_args_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/args/

paludis_args_include_HEADERS = \
	args.hh                \
	args_error.hh          \
	args_group.hh          \
	args_handler.hh        \
	args_handler-se.hh     \
	args_option.hh         \
	args_section.hh        \
	bad_argument.hh        \
	args_visitor.hh        \
	args_dumper.hh         \
	escape.hh              \
	man.hh                 \
	deps_option_arg.hh     \
	log_level_arg.hh       \
	dep_list_args_group.hh \
	install_args_group.hh  \
	do_help.hh

libpaludisargs_@PALUDIS_PC_SLOT@_la_SOURCES = \
	args.cc \
	args_error.cc \
	args_group.cc \
	args_handler.cc \
	args_option.cc \
	args_section.cc \
	bad_argument.cc \
	args_visitor.cc \
	args_dumper.cc \
	deps_option_arg.cc \
	escape.cc \
	log_level_arg.cc \
	dep_list_args_group.cc \
	install_args_group.cc

libpaludisman_@PALUDIS_PC_SLOT@_a_SOURCES = \
	man.cc

libpaludisargs_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisargs_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

TESTS_ENVIRONMENT = env PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_DISTRIBUTION="gentoo" \
	bash $(top_srcdir)/test/run_test.sh

lib_LTLIBRARIES = libpaludisargs_@PALUDIS_PC_SLOT@.la
noinst_LIBRARIES = libpaludisman_@PALUDIS_PC_SLOT@.a

check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/paludis/util/test_extras.o \
	libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)
args_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

args_handler-se.hh : args_handler.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/args_handler.se > $@ ; then rm -f $@ ; exit 1 ; fi

args_handler-se.cc : args_handler.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/args_handler.se > $@ ; then rm -f $@ ; exit 1 ; fi

