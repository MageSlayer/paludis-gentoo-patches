CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)

TESTS = \
	args_TEST

libpaludisargs_la_SOURCES = \
	args.hh          args.cc \
	args_error.hh    args_error.cc \
	args_group.hh    args_group.cc \
	args_handler.hh  args_handler.cc \
	args_option.hh   args_option.cc \
	bad_argument.hh  bad_argument.cc \
	args_visitor.hh  args_visitor.cc \
	bad_value.hh     bad_value.cc \
	missing_value.hh missing_value.cc \
	args_dumper.hh   args_dumper.cc

libpaludisargs_la_LDFLAGS = -version-info 0:0:0

TESTS_ENVIRONMENT = env -u PALUDIS_OPTIONS TEST_SCRIPT_DIR="$(srcdir)/" $(SHELL) $(top_srcdir)/test/run_test.sh
lib_LTLIBRARIES = libpaludisargs.la
check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/test/libtest.a \
	libpaludisargs.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)

