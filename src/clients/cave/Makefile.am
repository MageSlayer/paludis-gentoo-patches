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

# When adding commands, you need to add to both command_MANS and
# libcave_a_SOURCES. You should also update command_factory.cc,
# doc/clients/Makefile.am and .gitignore.

command_MANS = \
	cave-config.1 \
	cave-contents.1 \
	cave-digest.1 \
	cave-display-resolution.1 \
	cave-dump-cave-formats-conf.1 \
	cave-executables.1 \
	cave-execute-resolution.1 \
	cave-find-candidates.1 \
	cave-fix-cache.1 \
	cave-fix-linkage.1 \
	cave-graph-jobs.1 \
	cave-has-version.1 \
	cave-help.1 \
	cave-import.1 \
	cave-info.1 \
	cave-manage-search-index.1 \
	cave-match.1 \
	cave-owner.1 \
	cave-perform.1 \
	cave-print-best-version.1 \
	cave-print-categories.1 \
	cave-print-commands.1 \
	cave-print-environment-metadata.1 \
	cave-print-id-actions.1 \
	cave-print-id-contents.1 \
	cave-print-id-environment-variable.1 \
	cave-print-id-executables.1 \
	cave-print-id-masks.1 \
	cave-print-id-metadata.1 \
	cave-print-id-size.1 \
	cave-print-ids.1 \
	cave-print-owners.1 \
	cave-print-packages.1 \
	cave-print-repositories.1 \
	cave-print-repository-formats.1 \
	cave-print-repository-metadata.1 \
	cave-print-set.1 \
	cave-print-sets.1 \
	cave-print-spec.1 \
	cave-print-sync-protocols.1 \
	cave-print-unused-distfiles.1 \
	cave-purge.1 \
	cave-report.1 \
	cave-resolve.1 \
	cave-resume.1 \
	cave-search.1 \
	cave-show.1 \
	cave-size.1 \
	cave-sync.1 \
	cave-sync-protocol-options.1 \
	cave-uninstall.1 \
	cave-update-world.1 \
	cave-verify.1

man_MANS = \
	cave.1 \
	$(command_MANS)

cave.1 : man-cave
	env \
	    CAVE_FORMATS_CONF="/dev/null" \
	    CAVE_COMMANDS_PATH="" \
	    ./man-cave > $@

$(command_MANS) : man-cave
	if ! env \
	    CAVE_FORMATS_CONF="/dev/null" \
	    CAVE_COMMANDS_PATH="" \
	    ./man-cave `echo $@ | sed -e 's.^cave-..' -e 's,\.1$$,,'` > $@ ; then rm -f $@ ; exit 1 ; fi

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
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/resolver/libpaludisresolver.a

noinst_LIBRARIES = libcave.a

# See note above for adding commands
libcave_a_SOURCES = \
	colour_formatter.cc colour_formatter.hh colour_formatter-fmt.hh \
	colour_pretty_printer.cc colour_pretty_printer.hh colour_pretty_printer-fmt.hh \
	command.cc command.hh \
	command_line.cc command_line.hh \
	command_command_line.cc command_command_line.hh \
	command_factory.cc command_factory.hh \
	cmd_contents.cc cmd_contents.hh cmd_contents-fmt.hh \
	cmd_config.cc cmd_config.hh \
	cmd_digest.cc cmd_digest.hh \
	cmd_display_resolution.cc cmd_display_resolution.hh cmd_display_resolution-fmt.hh \
	cmd_dump_cave_formats_conf.cc cmd_dump_cave_formats_conf.hh \
	cmd_executables.cc cmd_executables.hh cmd_executables-fmt.hh \
	cmd_execute_resolution.cc cmd_execute_resolution.hh cmd_execute_resolution-fmt.hh \
	cmd_find_candidates.cc cmd_find_candidates.hh \
	cmd_fix_cache.cc cmd_fix_cache.hh cmd_fix_cache-fmt.hh \
	cmd_fix_linkage.cc cmd_fix_linkage.hh \
	cmd_graph_jobs.cc cmd_graph_jobs.hh \
	cmd_has_version.cc cmd_has_version.hh \
	cmd_help.cc cmd_help.hh \
	cmd_import.cc cmd_import.hh \
	cmd_info.cc cmd_info.hh cmd_info-fmt.hh \
	cmd_manage_search_index.cc cmd_manage_search_index.hh \
	cmd_match.cc cmd_match.hh \
	cmd_owner.cc cmd_owner.hh cmd_owner-fmt.hh \
	cmd_perform.cc cmd_perform.hh cmd_perform-fmt.hh \
	cmd_print_best_version.cc cmd_print_best_version.hh \
	cmd_print_categories.cc cmd_print_categories.hh \
	cmd_print_commands.cc cmd_print_commands.hh \
	cmd_print_environment_metadata.cc cmd_print_environment_metadata.hh \
	cmd_print_id_actions.cc cmd_print_id_actions.hh \
	cmd_print_id_contents.cc cmd_print_id_contents.hh \
	cmd_print_id_environment_variable.cc cmd_print_id_environment_variable.hh \
	cmd_print_id_executables.cc cmd_print_id_executables.hh \
	cmd_print_id_masks.cc cmd_print_id_masks.hh \
	cmd_print_id_metadata.cc cmd_print_id_metadata.hh \
	cmd_print_id_size.cc cmd_print_id_size.hh \
	cmd_print_ids.cc cmd_print_ids.hh \
	cmd_print_owners.cc cmd_print_owners.hh \
	cmd_print_packages.cc cmd_print_packages.hh \
	cmd_print_repositories.cc cmd_print_repositories.hh \
	cmd_print_repository_formats.cc cmd_print_repository_formats.hh \
	cmd_print_repository_metadata.cc cmd_print_repository_metadata.hh \
	cmd_print_set.cc cmd_print_set.hh \
	cmd_print_sets.cc cmd_print_sets.hh \
	cmd_print_spec.cc cmd_print_spec.hh \
	cmd_print_sync_protocols.cc cmd_print_sync_protocols.hh \
	cmd_print_unused_distfiles.cc cmd_print_unused_distfiles.hh \
	cmd_purge.cc cmd_purge.hh \
	cmd_report.cc cmd_report.hh cmd_report-fmt.hh \
	cmd_resolve.cc cmd_resolve.hh \
	cmd_resolve_display_callback.cc cmd_resolve_display_callback.hh \
	cmd_resolve_dump.cc cmd_resolve_dump.hh \
	cmd_resume.cc cmd_resume.hh \
	cmd_search.cc cmd_search.hh \
	cmd_search_cmdline.cc cmd_search_cmdline.hh \
	cmd_show.cc cmd_show.hh cmd_show-fmt.hh \
	cmd_size.cc cmd_size.hh \
	cmd_sync.cc cmd_sync.hh cmd_sync-fmt.hh \
	cmd_sync_protocol_options.cc cmd_sync_protocol_options.hh \
	cmd_uninstall.cc cmd_uninstall.hh \
	cmd_update_world.cc cmd_update_world.hh cmd_update_world-fmt.hh \
	cmd_verify.cc cmd_verify.hh cmd_verify-fmt.hh \
	colours.cc colours.hh \
	exceptions.cc exceptions.hh \
	executables_common.cc executables_common.hh \
	format_plain_contents_entry.cc format_plain_contents_entry.hh \
	format_plain_metadata_key.cc format_plain_metadata_key.hh \
	format_string.cc format_string.hh \
	format_user_config.cc format_user_config.hh \
	script_command.cc script_command.hh \
	search_extras_handle.cc search_extras_handle.hh \
	select_format_for_spec.cc select_format_for_spec.hh \
	owner_common.cc owner_common.hh \
	resolve_cmdline.cc resolve_cmdline.hh \
	resolve_common.cc resolve_common.hh \
	resume_data.cc resume_data.hh \
	size_common.cc size_common.hh

TESTS = \
	continue_on_failure_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	continue_on_failure_TEST_setup.sh continue_on_failure_TEST_cleanup.sh \
	moo

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

lib_LTLIBRARIES =

lib_LTLIBRARIES += libcavematchextras_@PALUDIS_PC_SLOT@.la

libcavematchextras_@PALUDIS_PC_SLOT@_la_SOURCES = match_extras.cc match_extras.hh
libcavematchextras_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) @PCRECPPDEPS_CFLAGS@
libcavematchextras_@PALUDIS_PC_SLOT@_la_LIBADD = @PCRECPPDEPS_LIBS@
libcavematchextras_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

if ENABLE_SEARCH_INDEX

lib_LTLIBRARIES += libcavesearchextras_@PALUDIS_PC_SLOT@.la

libcavesearchextras_@PALUDIS_PC_SLOT@_la_SOURCES = search_extras.cc search_extras.hh
libcavesearchextras_@PALUDIS_PC_SLOT@_la_CXXFLAGS = $(AM_CXXFLAGS) @SQLITE3DEPS_CFLAGS@
libcavesearchextras_@PALUDIS_PC_SLOT@_la_LIBADD = @SQLITE3DEPS_LIBS@
libcavesearchextras_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

endif

cavecommandsdir = $(libexecdir)/cave/commands
cavecommands_SCRIPTS = moo

