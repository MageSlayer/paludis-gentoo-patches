CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = run_test.sh

libtest_a_SOURCES = \
	test_framework.cc \
	test_framework.hh \
	test_runner.cc \
	test_runner.hh

libtestnomain_a_SOURCES = \
	test_framework.cc \
	test_framework.hh

XFAIL_TESTS = test_fail_TEST
TESTS = test_pass_TEST $(XFAIL_TESTS)

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh
check_LIBRARIES = libtest.a libtestnomain.a
check_PROGRAMS = $(TESTS) $(XFAIL_TESTS)
check_SCRIPTS = run_test.sh

test_pass_TEST_SOURCES = test_pass_TEST.cc
test_pass_TEST_LDADD = libtest.a

test_fail_TEST_SOURCES = test_fail_TEST.cc
test_fail_TEST_LDADD = libtest.a

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done
