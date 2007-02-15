AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = paludis
noinst_PROGRAMS = man-paludis
noinst_DATA = paludis.html
man_MANS = paludis.1

paludis.1 : man-paludis
	./man-paludis > $@

paludis.html : man-paludis
	./man-paludis --html > $@

man_paludis_SOURCES = \
	man_paludis.cc \
	command_line.hh \
	command_line.cc

man_paludis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludismanpagethings.la \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

paludis_SOURCES = \
	applets.hh           applets.cc           \
	command_line.hh      command_line.cc      \
	install.hh           install.cc           \
	uninstall.hh         uninstall.cc         \
	list.hh              list.cc              \
	query.hh             query.cc             \
	sync.hh              sync.cc              \
	do_contents.hh       do_contents.cc       \
	do_config.hh         do_config.cc         \
	owner.hh             owner.cc             \
	news.hh              news.cc              \
	report.hh            report.cc            \
	paludis.cc

if MONOLITHIC

paludis_LDADD = \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/default/libpaludisdefaultenvironment.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

else

paludis_LDADD = \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/default/libpaludisdefaultenvironment.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

endif

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_OPTIONS="" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST \
	list_repository_formats_TEST list_dep_tag_categories_TEST \
	exception_TEST install_TEST upgrade_TEST args_from_environment_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	install_TEST_setup.sh install_TEST_cleanup.sh \
	upgrade_TEST_setup.sh upgrade_TEST_cleanup.sh

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_MANS) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


