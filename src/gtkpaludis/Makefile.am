AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@

DEFS = \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

SUBDIRS = vtemm .

INCLUDES = $(DEPS_CFLAGS)

if ENABLE_GTK

bin_PROGRAMS = gtkpaludis

gtkpaludis_SOURCES = \
	command_line.cc       command_line.hh \
	browse_tree.cc        browse_tree.hh \
	information_tree.cc   information_tree.hh \
	vte_message_window.hh vte_message_window.cc \
	main_window.cc        main_window.hh \
	sync.cc               sync.hh \
	gtkpaludis.cc

gtkpaludis_LDADD = \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/gtkpaludis/vtemm/libvtemm.a \
	$(DEPS_LIBS) \
	$(DYNAMIC_LD_LIBS)

endif

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in


