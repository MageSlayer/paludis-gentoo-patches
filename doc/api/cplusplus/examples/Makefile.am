CLEANFILES = *~ *.epicfail
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
	example_dep_spec_flattener \
	example_environment \
	example_package_id \
	example_metadata_key \
	example_mask \
	example_repository \
	example_match_package \
	example_package_database \
	example_selection \
	example_version_operator \
	example_version_spec \
	example_name

EXTRA_DIST = $(noinst_SCRIPTS)

noinst_LIBRARIES = libpaludisexamples.a

libpaludisexamples_a_SOURCES = \
	example_command_line.hh \
	example_command_line.cc

libpaludisexamples_a_LIBADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_about_SOURCES = example_about.cc
example_about_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la

example_action_SOURCES = example_action.cc
example_action_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_contents_SOURCES = example_contents.cc
example_contents_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_dep_label_SOURCES = example_dep_label.cc
example_dep_label_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_dep_spec_SOURCES = example_dep_spec.cc
example_dep_spec_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_dep_spec_flattener_SOURCES = example_dep_spec_flattener.cc
example_dep_spec_flattener_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_environment_SOURCES = example_environment.cc
example_environment_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_package_id_SOURCES = example_package_id.cc
example_package_id_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_mask_SOURCES = example_mask.cc
example_mask_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_metadata_key_SOURCES = example_metadata_key.cc
example_metadata_key_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_repository_SOURCES = example_repository.cc
example_repository_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_match_package_SOURCES = example_match_package.cc
example_match_package_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_selection_SOURCES = example_selection.cc
example_selection_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_package_database_SOURCES = example_package_database.cc
example_package_database_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_version_operator_SOURCES = example_version_operator.cc
example_version_operator_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_version_spec_SOURCES = example_version_spec.cc
example_version_spec_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

example_name_SOURCES = example_name.cc
example_name_LDADD = \
	libpaludisexamples.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

htmldocs : htmldocs-subdirs

htmldocs-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) htmldocs-local || exit 1 ; \
	    else $(MAKE) -C $$s htmldocs || exit 1 ; fi ; done

htmldocs-local : $(paludisapicplusplusexampleshtml_DATA)

