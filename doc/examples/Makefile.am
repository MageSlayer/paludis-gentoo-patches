CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

noinst_PROGRAMS = \
	example_about \
	example_action \
	example_contents \
	example_dep_label

EXTRA_DIST = $(noinst_SCRIPTS)

example_about_SOURCES = example_about.cc
example_about_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la

example_action_SOURCES = example_action.cc
example_action_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

example_contents_SOURCES = example_contents.cc
example_contents_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

example_dep_label_SOURCES = example_dep_label.cc
example_dep_label_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

