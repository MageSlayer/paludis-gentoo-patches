CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = run_test.sh

AUTOMAKE_OPTIONS = parallel-tests

libtest_a_SOURCES = \
	test_framework.cc \
	test_framework.hh \
	test_runner.cc \
	test_runner.hh \
	test_concepts.hh

libtestnomain_a_SOURCES = \
	test_framework.cc \
	test_framework.hh \
	test_concepts.hh

XFAIL_TESTS = test_fail_TEST
TESTS = test_pass_TEST $(XFAIL_TESTS)

TESTS_ENVIRONMENT = env PALUDIS_OPTIONS="" \
		    TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh
check_LIBRARIES = libtest.a libtestnomain.a
check_PROGRAMS = $(TESTS) $(XFAIL_TESTS)
check_SCRIPTS = run_test.sh

test_pass_TEST_SOURCES = test_pass_TEST.cc
test_pass_TEST_LDADD = libtest.a
test_pass_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

test_fail_TEST_SOURCES = test_fail_TEST.cc
test_fail_TEST_LDADD = libtest.a
test_fail_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

built-sources : $(BUILT_SOURCES)
	for s in $(SUBDIRS) ; do $(MAKE) -C $$s built-sources || exit 1 ; done

