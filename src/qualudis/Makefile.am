CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = .

if ENABLE_QA

bin_PROGRAMS = qualudis

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

qualudis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.a \
	$(top_builddir)/paludis/qa/libpaludisqa.a \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/util/libpaludisutil.a

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh bash
TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

endif

