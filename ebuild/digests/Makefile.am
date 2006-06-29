MAINTAINERCLEANFILES = Makefile.in
CLEANFILES = *~
SUBDIRS = .

libexecprogdir = $(libexecdir)/paludis/digests/

libexecprog_PROGRAMS = \
	dosha256 \
	dormd160 \
	domd5

dosha256_SOURCES = dosha256.cc
dosha256_LDADD = $(top_builddir)/paludis/digests/libpaludisdigests.a

dormd160_SOURCES = dormd160.cc
dormd160_LDADD = $(top_builddir)/paludis/digests/libpaludisdigests.a

domd5_SOURCES = domd5.cc
domd5_LDADD = $(top_builddir)/paludis/digests/libpaludisdigests.a

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(srcdir)/ebuild/" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

AM_CXXFLAGS = -I$(top_srcdir)
