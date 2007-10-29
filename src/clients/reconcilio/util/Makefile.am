AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

noinst_LIBRARIES = libreconcilioutil.a

libreconcilioutil_a_SOURCES = \
	realpath.cc          realpath.hh \
	wildcard_expander.cc wildcard_expander.hh \
	byte_swap.hh

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = realpath_TEST wildcard_expander_TEST byte_swap_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	realpath_TEST_setup.sh realpath_TEST_cleanup.sh \
	wildcard_expander_TEST_setup.sh wildcard_expander_TEST_cleanup.sh

realpath_TEST_SOURCES = realpath_TEST.cc
realpath_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

wildcard_expander_TEST_SOURCES = wildcard_expander_TEST.cc
wildcard_expander_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

byte_swap_TEST_SOURCES = byte_swap_TEST.cc
byte_swap_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
