include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DBINDIR=\"$(bindir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DPYTHONINSTALLDIR=\"$(PYTHON_INSTALL_DIR)\" \
	-DRUBYINSTALLDIR=\"$(RUBY_INSTALL_DIR)\"

SUBDIRS = .

bin_PROGRAMS = cave

noinst_PROGRAMS = man-cave

command_MANS = \
	cave-config.1 \
	cave-display-resolution.1 \
	cave-execute-resolution.1 \
	cave-find-candidates.1 \
	cave-fix-cache.1 \
	cave-help.1 \
	cave-import.1 \
	cave-info.1 \
	cave-perform.1 \
	cave-print-categories.1 \
	cave-print-commands.1 \
	cave-print-environment-metadata.1 \
	cave-print-id-contents.1 \
	cave-print-id-executables.1 \
	cave-print-id-metadata.1 \
	cave-print-ids.1 \
	cave-print-owners.1 \
	cave-print-packages.1 \
	cave-print-repositories.1 \
	cave-print-repository-formats.1 \
	cave-print-sets.1 \
	cave-print-sync-protocols.1 \
	cave-resolve.1 \
	cave-show.1 \
	cave-sync.1 \
	cave-update-world.1

man_MANS = \
	cave.1 \
	$(command_MANS)

cave.1 : man-cave
	./man-cave > $@

$(command_MANS) : man-cave
	if ! ./man-cave `echo $@ | sed -e 's.^cave-..' -e 's,\.1$$,,'` > $@ ; then rm -f $@ ; exit 1 ; fi

cave_SOURCES = \
	cave.cc

cave_LDADD = \
	libcave.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/resolver/libpaludisresolver.a \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

man_cave_SOURCES = \
	man_cave.cc

man_cave_LDADD = \
	libcave.a \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/resolver/libpaludisresolver.a

noinst_LIBRARIES = libcave.a

libcave_a_SOURCES = \
	colour_formatter.cc colour_formatter.hh \
	command.cc command.hh \
	command_line.cc command_line.hh \
	command_command_line.cc command_command_line.hh \
	command_factory.cc command_factory.hh \
	cmd_config.cc cmd_config.hh \
	cmd_display_resolution.cc cmd_display_resolution.hh \
	cmd_execute_resolution.cc cmd_execute_resolution.hh \
	cmd_find_candidates.cc cmd_find_candidates.hh \
	cmd_fix_cache.cc cmd_fix_cache.hh \
	cmd_help.cc cmd_help.hh \
	cmd_import.cc cmd_import.hh \
	cmd_info.cc cmd_info.hh \
	cmd_match.cc cmd_match.hh \
	cmd_perform.cc cmd_perform.hh \
	cmd_print_categories.cc cmd_print_categories.hh \
	cmd_print_commands.cc cmd_print_commands.hh \
	cmd_print_environment_metadata.cc cmd_print_environment_metadata.hh \
	cmd_print_id_contents.cc cmd_print_id_contents.hh \
	cmd_print_id_executables.cc cmd_print_id_executables.hh \
	cmd_print_id_metadata.cc cmd_print_id_metadata.hh \
	cmd_print_ids.cc cmd_print_ids.hh \
	cmd_print_owners.cc cmd_print_owners.hh \
	cmd_print_packages.cc cmd_print_packages.hh \
	cmd_print_repositories.cc cmd_print_repositories.hh \
	cmd_print_repository_formats.cc cmd_print_repository_formats.hh \
	cmd_print_sets.cc cmd_print_sets.hh \
	cmd_print_sync_protocols.cc cmd_print_sync_protocols.hh \
	cmd_resolve.cc cmd_resolve.hh \
	cmd_resolve_cmdline.cc cmd_resolve_cmdline.hh \
	cmd_resolve_display_callback.cc cmd_resolve_display_callback.hh \
	cmd_resolve_dump.cc cmd_resolve_dump.hh \
	cmd_search_cmdline.cc cmd_search_cmdline.hh \
	cmd_show.cc cmd_show.hh \
	cmd_sync.cc cmd_sync.hh \
	cmd_update_world.cc cmd_update_world.hh \
	exceptions.cc exceptions.hh \
	format_general.cc format_general.hh \
	format_plain_contents_entry.cc format_plain_contents_entry.hh \
	format_plain_metadata_key.cc format_plain_metadata_key.hh \
	format_string.cc format_string.hh \
	formats.cc formats.hh \
	select_format_for_spec.cc select_format_for_spec.hh \
	match_qpns.cc match_qpns.hh \
	resolve_common.cc resolve_common.hh

#	cmd_contents.cc \
#	cmd_executables.cc \
#	cmd_owner.cc \
#	cmd_print_id_actions.cc \
#	cmd_print_id_masks.cc \
#	cmd_print_repository_metadata.cc \
#	cmd_print_set.cc \
#	cmd_print_sync_protocol_metadata.cc \
#	cmd_report.cc \
#	cmd_uninstall.cc \
#	cmd_uninstall_unused.cc

TESTS =

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS)

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

