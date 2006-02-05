CLEANFILES = *~ version_TEST
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir)
DEFS=-DSYSCONFDIR=\"$(sysconfdir)\" -DLIBEXECDIR=\"$(libexecdir)\"

bin_PROGRAMS = paludis
noinst_PROGRAMS = check_deps_exist find_self_circular_dependencies

paludis_SOURCES = \
	command_line.hh      command_line.cc      \
	colour.hh            colour.cc            \
	install.hh           install.cc           \
	list.hh              list.cc              \
	query.hh             query.cc             \
	paludis.cc

check_deps_exist_SOURCES = \
	check_deps_exist.cc

find_self_circular_dependencies_SOURCES = \
	find_self_circular_dependencies.cc

paludis_LDADD = \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/args/libpaludisargs.a

check_deps_exist_LDADD = \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/args/libpaludisargs.a

find_self_circular_dependencies_LDADD = \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/args/libpaludisargs.a

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" $(SHELL) $(top_srcdir)/test/run_test.sh bash
TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./paludis --version" > $@

