CLEANFILES = *~ version_TEST
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)
DEFS=-DSYSCONFDIR=\"$(sysconfdir)\" -DLIBEXECDIR=\"$(libexecdir)\"
SUBDIRS = .

bin_PROGRAMS = qualudis

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

qualudis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.a \
	$(top_builddir)/paludis/qa/libpaludisqa.a \
	$(top_builddir)/paludis/util/libpaludisutil.a \
	$(top_builddir)/paludis/libpaludis.la

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" $(SHELL) $(top_srcdir)/test/run_test.sh bash
TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

