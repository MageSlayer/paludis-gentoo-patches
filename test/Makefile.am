include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = run_test.sh

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

my_XFAIL_TESTS = test_fail_TEST
XFAIL_TESTS = $(my_XFAIL_TESTS)
TESTS = test_pass_TEST $(my_XFAIL_TESTS)

check_LIBRARIES = libtest.a libtestnomain.a
check_PROGRAMS = $(TESTS) $(XFAIL_TESTS)
check_SCRIPTS = run_test.sh

test_pass_TEST_SOURCES = test_pass_TEST.cc
test_pass_TEST_LDADD = libtest.a
test_pass_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

test_fail_TEST_SOURCES = test_fail_TEST.cc
test_fail_TEST_LDADD = libtest.a
test_fail_TEST_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

