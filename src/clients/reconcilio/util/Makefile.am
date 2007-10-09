AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

SUBDIRS = .

noinst_LIBRARIES = libreconcilioutil.a

libreconcilioutil_a_SOURCES = \
	realpath.cc          realpath.hh \
	wildcard_expander.cc wildcard_expander.hh \
	iterator.hh

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = iterator_TEST realpath_TEST wildcard_expander_TEST
check_PROGRAMS = $(TESTS)

EXTRA_DIST = \
	$(TESTS) \
	realpath_TEST_setup.sh realpath_TEST_cleanup.sh \
	wildcard_expander_TEST_setup.sh wildcard_expander_TEST_cleanup.sh

iterator_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

realpath_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

wildcard_expander_TEST_LDADD = \
	libreconcilioutil.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


