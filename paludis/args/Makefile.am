CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)

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

libpaludisargs_a_SOURCES = $(paludis_args_include_HEADERS) \
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

TESTS_ENVIRONMENT = env -u PALUDIS_OPTIONS TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh
lib_LIBRARIES = libpaludisargs.a
check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/test/libtest.a \
	libpaludisargs.a \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/util/libpaludisutil.a

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)

