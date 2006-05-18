MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/digests/

libexecprog_SCRIPTS = \
	domd5

libexecprog_PROGRAMS = \
	dosha256 \
	dormd160

dosha256_SOURCES = \
	sha256.cc \
	sha256.hh \
	dosha256.cc

dormd160_SOURCES = \
	rmd160.cc \
	rmd160.hh \
	dormd160.cc

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

TESTS = dosha256_TEST dormd160_TEST
EXTRA_DIST = $(libexecprog_SCRIPTS) dosha256_TEST.cc dormd160_TEST.cc

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -I$(top_srcdir)

dosha256_TEST_SOURCES = sha256.cc sha256.hh dosha256_TEST.cc
dosha256_TEST_CXXFLAGS = -I$(top_srcdir)
dosha256_TEST_LDADD = $(top_builddir)/test/libtest.a

dormd160_TEST_SOURCES = rmd160.cc rmd160.hh dormd160_TEST.cc
dormd160_TEST_CXXFLAGS = -I$(top_srcdir)
dormd160_TEST_LDADD = $(top_builddir)/test/libtest.a

