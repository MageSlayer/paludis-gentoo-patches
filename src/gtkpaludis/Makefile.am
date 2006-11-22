AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	@PALUDIS_CXXFLAGS_NO_WSHADOW@

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

SUBDIRS = cellrendererbutton vtemm .

INCLUDES = $(GTKDEPS_CFLAGS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_HOME="$(srcdir)" \
	SYSCONFDIR="$(sysconfdir)" \
	GNOME_ACCESSIBILITY="1" \
	GTK_MODULES="gail:atk-bridge" \
	bash $(top_srcdir)/test/run_test.sh $(srcdir)/test_helper.bash

IF_GTK_TESTS = version_TEST help_TEST sets_list_TEST

if ENABLE_GTK

bin_PROGRAMS = gtkpaludis

common_sources = \
	categories_list.cc     categories_list.hh     \
	install.cc             install.hh             \
	main_window.cc         main_window.hh         \
	menu.cc                menu.hh                \
	messages.cc            messages.hh            \
	package_info.cc        package_info.hh        \
	package_overview.cc    package_overview.hh    \
	packages_list.cc       packages_list.hh       \
	packages_page.cc       packages_page.hh       \
	paludis_thread.cc      paludis_thread.hh      \
	queue_list.cc          queue_list.hh          \
	queue_options.cc       queue_options.hh       \
	queue_page.cc          queue_page.hh          \
	repositories_list.cc   repositories_list.hh   \
	repositories_page.cc   repositories_page.hh   \
	repository_overview.cc repository_overview.hh \
	set_overview.cc        set_overview.hh        \
	sets_list.cc           sets_list.hh           \
	sets_page.cc           sets_page.hh           \
	sync.cc                sync.hh                \
	tasks_page.cc          tasks_page.hh

gtkpaludis_SOURCES = \
	$(common_sources) \
	command_line.cc        command_line.hh        \
	gtkpaludis.cc

gtkpaludis_LDADD = \
	$(top_builddir)/src/gtkpaludis/vtemm/libvtemm.a \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/default/libpaludisdefaultenvironment.la \
	$(GTKDEPS_LIBS) \
	$(DYNAMIC_LD_LIBS)

BUILT_SOURCES = packages_list-sr.hh packages_list-sr.cc

if ENABLE_GTK_TESTS

check_PROGRAMS = prod-x-server sets_list_TEST
check_SCRIPTS = \
	sets_list_TEST_setup.sh sets_list_TEST_cleanup.sh \
	quit_TEST.py quit_TEST_setup.sh quit_TEST_cleanup.sh

check_LIBRARIES = libtestcommon.a

libtestcommon_a_SOURCES = test_common.hh test_common.cc

sets_list_TEST_SOURCES = $(common_sources) sets_list_TEST.cc
sets_list_TEST_LDADD = \
	libtestcommon.a \
	$(gtkpaludis_LDADD) \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtestnomain.a

prod_x_server_SOURCES = prod-x-server.cc
prod_x_server_LDADD = -lX11

TESTS = version_TEST help_TEST quit_TEST.py sets_list_TEST

endif

endif

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = packages_list-sr.hh packages_list-sr.cc
EXTRA_DIST = packages_list.sr version_TEST help_TEST $(check_SCRIPTS)

packages_list-sr.hh : packages_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/packages_list.sr > $@

packages_list-sr.cc : packages_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/packages_list.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


