AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = . qualudis

bin_PROGRAMS = paludis

paludis_SOURCES = \
	applets.hh           applets.cc           \
	command_line.hh      command_line.cc      \
	colour.hh            colour.cc            \
	install.hh           install.cc           \
	uninstall.hh         uninstall.cc         \
	list.hh              list.cc              \
	query.hh             query.cc             \
	sync.hh              sync.cc              \
	licence.hh           licence.cc           \
	paludis.cc

paludis_LDADD = \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/args/libpaludisargs.a \
	$(top_builddir)/paludis/util/libpaludisutil.a

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_SKIP_CONFIG="yes" \
	$(SHELL) $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST list_sync_formats_TEST \
	list_repository_formats_TEST list_dep_tag_categories_TEST

CLEANFILES = *~ $(TESTS) gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

version_TEST : paludis
	echo -e "#!/bin/sh\n./paludis --version" > $@

help_TEST : paludis
	echo -e "#!/bin/sh\n./paludis --help" > $@

list_sync_formats_TEST : paludis
	echo -e "#!/bin/sh\n./paludis --list-sync-protocols" > $@

list_repository_formats_TEST : paludis
	echo -e "#!/bin/sh\n./paludis --list-repository-formats" > $@

list_dep_tag_categories_TEST : paludis
	echo -e "#!/bin/sh\n./paludis --list-dep-tag-categories" > $@

