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
	example_dep_label \
	example_dep_spec \
	example_dep_tag \
	example_dep_tree \
	example_dep_spec_flattener \
	example_environment \
	example_formatter \
	example_stringify_formatter \
	example_package_id \
	example_metadata_key \
	example_mask \
	example_repository

EXTRA_DIST = $(noinst_SCRIPTS)

noinst_LIBRARIES = libpaludisexamples.a

libpaludisexamples_a_SOURCES = \
	example_command_line.hh \
	example_command_line.cc

libpaludisexamples_a_LIBADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_about_SOURCES = example_about.cc
example_about_LDFLAGS = \
	$(top_builddir)/paludis/libpaludis.la

example_action_SOURCES = example_action.cc
example_action_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_contents_SOURCES = example_contents.cc
example_contents_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_dep_label_SOURCES = example_dep_label.cc
example_dep_label_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_dep_spec_SOURCES = example_dep_spec.cc
example_dep_spec_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_dep_tree_SOURCES = example_dep_tree.cc
example_dep_tree_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_dep_tag_SOURCES = example_dep_tag.cc
example_dep_tag_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_dep_spec_flattener_SOURCES = example_dep_spec_flattener.cc
example_dep_spec_flattener_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_environment_SOURCES = example_environment.cc
example_environment_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_formatter_SOURCES = example_formatter.cc
example_formatter_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_stringify_formatter_SOURCES = example_stringify_formatter.cc
example_stringify_formatter_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_package_id_SOURCES = example_package_id.cc
example_package_id_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_mask_SOURCES = example_mask.cc
example_mask_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_metadata_key_SOURCES = example_metadata_key.cc
example_metadata_key_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

example_repository_SOURCES = example_repository.cc
example_repository_LDFLAGS = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

