CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@

TESTS = \
	args_TEST

paludis_args_includedir = $(includedir)/paludis/args/

paludis_args_include_HEADERS = \
	args.hh           \
	args_error.hh     \
	args_group.hh     \
	args_handler.hh   \
	args_option.hh    \
	bad_argument.hh   \
	args_visitor.hh   \
	bad_value.hh      \
	missing_value.hh  \
	args_dumper.hh

libpaludisargs_la_SOURCES = $(paludis_args_include_HEADERS) \
	args.cc \
	args_error.cc \
	args_group.cc \
	args_handler.cc \
	args_option.cc \
	bad_argument.cc \
	args_visitor.cc \
	bad_value.cc \
	missing_value.cc \
	args_dumper.cc

libpaludisargs_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

TESTS_ENVIRONMENT = env -u PALUDIS_OPTIONS TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh
lib_LTLIBRARIES = libpaludisargs.la
check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/test/libtest.a \
	libpaludisargs.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)

