AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

noinst_LIBRARIES = libreconcilioutil.a

AUTOMAKE_OPTIONS = parallel-tests

libreconcilioutil_a_SOURCES = \
	realpath.cc          realpath.hh \
	wildcard_expander.cc wildcard_expander.hh

TESTS_ENVIRONMENT = env \
	PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = realpath_TEST wildcard_expander_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	realpath_TEST_setup.sh realpath_TEST_cleanup.sh \
	wildcard_expander_TEST_setup.sh wildcard_expander_TEST_cleanup.sh

realpath_TEST_SOURCES = realpath_TEST.cc
realpath_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

realpath_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

wildcard_expander_TEST_SOURCES = wildcard_expander_TEST.cc
wildcard_expander_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

wildcard_expander_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

