include $(top_srcdir)/misc/common-makefile.am

SUBDIRS = .

DISTCLEANFILES = args_handler-se.hh args_handler-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = args_handler-se.hh args_handler-se.cc args_handler.se
BUILT_SOURCES = args_handler-se.hh args_handler-se.cc

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
	log_level_arg.hh       \
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
	escape.cc \
	log_level_arg.cc \
	man.cc

libpaludisargs_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisargs_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludisargs_@PALUDIS_PC_SLOT@.la

check_PROGRAMS = $(TESTS)

test_ldadd = \
	$(top_builddir)/paludis/util/test_extras.o \
	libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

args_TEST_SOURCES = args_TEST.cc
args_TEST_LDADD = $(test_ldadd)
args_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

args_handler-se.hh : args_handler.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/args_handler.se > $@ ; then rm -f $@ ; exit 1 ; fi

args_handler-se.cc : args_handler.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/args_handler.se > $@ ; then rm -f $@ ; exit 1 ; fi

