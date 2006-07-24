AM_CXXFLAGS = -I$(top_srcdir) \
	@PALUDIS_CXXFLAGS@ \
	@PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ \
	@PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@

DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DGLIBMM_EXCEPTIONS_ENABLED=1

SUBDIRS = .

if ENABLE_GTK

bin_PROGRAMS = gtkpaludis

gtkpaludis_SOURCES = \
	command_line.cc     command_line.hh \
	browse_tree.cc      browse_tree.hh \
	information_tree.cc information_tree.hh \
	message_window.cc   message_window.hh \
	main_window.cc      main_window.hh \
	gtkpaludis.cc

gtkpaludis_LDADD = \
	$(top_builddir)/paludis/repositories/nothing/libpaludisnothingrepository.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/paludis/repositories/vdb/libpaludisvdbrepository.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(DEPS_LIBS)

INCLUDES = $(DEPS_CFLAGS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in

endif

