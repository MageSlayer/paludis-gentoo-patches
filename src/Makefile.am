CLEANFILES = *~ version_TEST
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = . qualudis

bin_PROGRAMS = paludis

paludis_SOURCES = \
	command_line.hh      command_line.cc      \
	colour.hh            colour.cc            \
	install.hh           install.cc           \
	list.hh              list.cc              \
	query.hh             query.cc             \
	paludis.cc

paludis_LDADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" $(SHELL) $(top_srcdir)/test/run_test.sh bash
TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./paludis --version" > $@

