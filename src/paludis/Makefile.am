AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\"
SUBDIRS = .

bin_PROGRAMS = paludis
noinst_PROGRAMS = man-paludis

man_MANS = paludis.1

paludis.1 : man-paludis
	./man-paludis | tee $@ | sed -e 's/^/    /'

man_paludis_SOURCES = \
	man_paludis.cc \
	command_line.hh \
	command_line.cc

man_paludis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

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
	$(top_builddir)/paludis/repositories/nothing/libpaludisnothingrepository.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/paludis/repositories/vdb/libpaludisvdbrepository.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST list_sync_formats_TEST \
	list_repository_formats_TEST list_dep_tag_categories_TEST \
	exception_TEST install_TEST upgrade_TEST

EXTRA_DIST = $(TESTS) \
	install_TEST_setup.sh install_TEST_cleanup.sh \
	upgrade_TEST_setup.sh upgrade_TEST_cleanup.sh

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda $(man_MANS)
MAINTAINERCLEANFILES = Makefile.in

