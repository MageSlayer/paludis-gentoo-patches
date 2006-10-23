AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ \
	@PALUDIS_CXXFLAGS_NO_WSHADOW@

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

SUBDIRS = vtemm .

INCLUDES = $(GTKDEPS_CFLAGS)

if ENABLE_GTK

bin_PROGRAMS = gtkpaludis

gtkpaludis_SOURCES = \
	categories_list.cc     categories_list.hh     \
	command_line.cc        command_line.hh        \
	main_window.cc         main_window.hh         \
	messages.cc            messages.hh            \
	package_info.cc        package_info.hh        \
	package_overview.cc    package_overview.hh    \
	packages_list.cc       packages_list.hh       \
	packages_page.cc       packages_page.hh       \
	paludis_thread.cc      paludis_thread.hh      \
	repositories_list.cc   repositories_list.hh   \
	repositories_page.cc   repositories_page.hh   \
	repository_overview.cc repository_overview.hh \
	sync.cc                sync.hh                \
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

endif

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in


