MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/digests/

libexecprog_SCRIPTS = \
	domd5 \
	dormd160

libexecprog_PROGRAMS = \
	dosha256

dosha256_SOURCES = \
	sha256.cc \
	sha256.hh \
	dosha256.cc

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	$(SHELL) $(top_srcdir)/test/run_test.sh

TESTS = dosha256_TEST
EXTRA_DIST = $(libexecprog_SCRIPTS) dosha256_TEST.cc

check_SCRIPTS =
check_PROGRAMS = $(TESTS)

dosha256_TEST_SOURCES = sha256.cc sha256.hh dosha256_TEST.cc
dosha256_TEST_CXXFLAGS = -I$(top_srcdir)
dosha256_TEST_LDADD = $(top_builddir)/test/libtest.a

