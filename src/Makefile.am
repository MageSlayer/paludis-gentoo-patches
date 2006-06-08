AM_CXXFLAGS = -I$(top_srcdir)
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\"
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
	contents.hh          contents.cc          \
	owner.hh             owner.cc             \
	news.hh              news.cc              \
	paludis.cc

paludis_LDADD = \
	$(top_builddir)/paludis/libpaludis.a \
	$(top_builddir)/paludis/args/libpaludisargs.a \
	$(top_builddir)/paludis/util/libpaludisutil.a

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST list_sync_formats_TEST \
	list_repository_formats_TEST list_dep_tag_categories_TEST \
	exception_TEST install_TEST

EXTRA_DIST = install_TEST_setup.sh install_TEST_cleanup.sh

CLEANFILES = *~ $(TESTS) gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

version_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_SKIP_CONFIG=yes ./paludis --version" > $@

help_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_SKIP_CONFIG=yes ./paludis --help" > $@

exception_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_HOME=./ \\" > $@
	echo -e "    ./paludis --config-suffix paludis-build-test -pi asdf 2>&1 \\" >> $@
	echo -e "    | grep 'Default configuration error' >/dev/null" >> $@

list_sync_formats_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_SKIP_CONFIG=yes ./paludis --list-sync-protocols" > $@

list_repository_formats_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_SKIP_CONFIG=yes ./paludis --list-repository-formats" > $@

list_dep_tag_categories_TEST : paludis
	echo -e "#!/bin/sh\nenv PALUDIS_SKIP_CONFIG=yes ./paludis --list-dep-tag-categories" > $@

install_TEST : paludis
	echo -e "#!/bin/sh\nenv \\" > $@
	echo -e "    PALUDIS_HOME=./install_TEST_dir/config/ \\" >> $@
	echo -e "   ./paludis --config-suffix install-test --install target" >> $@
	echo -e "./install_TEST_dir/root/usr/bin/testbin | grep success >/dev/null" >> $@

