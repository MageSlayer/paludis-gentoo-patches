CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)

TESTS = \
	args_TEST

libpaludisargs_a_SOURCES = \
	alias_arg.hh    alias_arg.cc \
	args.hh         args.cc \
	args_error.hh   args_error.cc \
	args_group.hh   args_group.cc \
	args_handler.hh args_handler.cc \
	args_option.hh  args_option.cc \
	bad_argument.hh bad_argument.cc \
	switch_arg.hh   switch_arg.cc \
	args_visitor.hh args_visitor.cc \
	bad_value.hh	bad_value.cc

noinst_LIBRARIES = libpaludisargs.a
check_PROGRAMS = $(TESTS)

test_ldadd = \
	libpaludisargs.a \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.a

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)

