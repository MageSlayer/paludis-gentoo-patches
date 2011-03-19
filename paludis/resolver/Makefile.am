include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES = \
	any_child_score-se.hh any_child_score-se.cc \
	change_type-se.hh change_type-se.cc \
	destination_types-se.hh destination_types-se.cc \
	job_requirements-se.hh job_requirements-se.cc \
	nag-se.hh nag-se.cc \
	resolver_functions-se.hh resolver_functions-se.cc \ \
	use_existing-se.hh use_existing-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = \
	any_child_score-se.hh any_child_score-se.cc any_child_score.se \
	change_type-se.hh change_type-se.cc change_type.se \
	destination_types-se.hh destination_types-se.cc destination_types.se \
	job_requirements-se.hh job_requirements-se.cc job_requirements.se \
	nag-se.hh nag-se.cc nag.se \
	resolver_functions-se.hh resolver_functions-se.cc resolver_functions.se \
	use_existing-se.hh use_existing-se.cc use_existing.se \
	$(check_SCRIPTS)
BUILT_SOURCES = \
	any_child_score-se.hh any_child_score-se.cc \
	change_type-se.hh change_type-se.cc \
	destination_types-se.hh destination_types-se.cc \
	nag-se.hh nag-se.cc \
	job_requirements-se.hh job_requirements-se.cc \
	resolver_functions-se.hh resolver_functions-se.cc \
	use_existing-se.hh use_existing-se.cc

noinst_HEADERS = \
	accumulate_deps_and_provides.hh accumulate_deps_and_provides-fwd.hh \
	allow_choice_changes_helper.hh allow_choice_changes_helper-fwd.hh \
	allowed_to_remove_helper.hh allowed_to_remove_helper-fwd.hh \
	allowed_to_restart_helper.hh allowed_to_restart_helper-fwd.hh \
	always_via_binary_helper.hh always_via_binary_helper-fwd.hh \
	any_child_score.hh any_child_score-fwd.hh any_child_score-se.hh \
	can_use_helper.hh can_use_helper-fwd.hh \
	change_by_resolvent.hh change_by_resolvent-fwd.hh \
	change_type.hh change_type-fwd.hh change_type-se.hh \
	collect_depped_upon.hh collect_depped_upon-fwd.hh \
	collect_installed.hh collect_installed-fwd.hh \
	collect_provided.hh collect_provided-fwd.hh \
	collect_purges.hh collect_purges-fwd.hh \
	collect_world.hh collect_world-fwd.hh \
	confirm_helper.hh confirm_helper-fwd.hh \
	constraint.hh constraint-fwd.hh \
	decider.hh decider-fwd.hh \
	decision.hh decision-fwd.hh \
	decision_utils.hh decision_utils-fwd.hh \
	decisions.hh decisions-fwd.hh \
	destination.hh destination-fwd.hh \
	destination_types.hh destination_types-fwd.hh destination_types-se.hh \
	destination_utils.hh destination_utils-fwd.hh \
	find_replacing_helper.hh find_replacing_helper-fwd.hh \
	find_repository_for_helper.hh find_repository_for_helper-fwd.hh \
	get_constraints_for_dependent_helper.hh get_constraints_for_dependent_helper-fwd.hh \
	get_constraints_for_purge_helper.hh get_constraints_for_purge_helper-fwd.hh \
	get_constraints_for_via_binary_helper.hh get_constraints_for_via_binary_helper-fwd.hh \
	get_destination_types_for_blocker_helper.hh get_destination_types_for_blocker_helper-fwd.hh \
	get_destination_types_for_error_helper.hh get_destination_types_for_error_helper-fwd.hh \
	get_initial_constraints_for_helper.hh get_initial_constraints_for_helper-fwd.hh \
	get_resolvents_for_helper.hh get_resolvents_for_helper-fwd.hh \
	get_use_existing_nothing_helper.hh get_use_existing_nothing_helper-fwd.hh \
	interest_in_spec_helper.hh interest_in_spec_helper-fwd.hh \
	job.hh job-fwd.hh \
	job_list.hh job_list-fwd.hh \
	job_lists.hh job_lists-fwd.hh \
	job_requirements.hh job_requirements-fwd.hh \
	job_state.hh job_state-fwd.hh \
	labels_classifier.hh labels_classifier-fwd.hh \
	make_destination_filtered_generator_helper.hh make_destination_filtered_generator_helper-fwd.hh \
	make_origin_filtered_generator_helper.hh make_origin_filtered_generator_helper-fwd.hh \
	make_uninstall_blocker.hh make_uninstall_blocker-fwd.hh \
	make_unmaskable_filter_helper.hh make_unmaskable_filter_helper-fwd.hh \
	match_qpns.hh match_qpns-fwd.hh \
	nag.hh nag-fwd.hh \
	order_early_helper.hh order_early_helper-fwd.hh \
	orderer.hh orderer-fwd.hh \
	orderer_notes.hh orderer_notes-fwd.hh \
	package_or_block_dep_spec.hh package_or_block_dep_spec-fwd.hh \
	prefer_or_avoid_helper.hh prefer_or_avoid_helper-fwd.hh \
	reason.hh reason-fwd.hh \
	reason_utils.hh reason_utils-fwd.hh \
	remove_if_dependent_helper.hh remove_if_dependent_helper-fwd.hh \
	required_confirmations.hh required_confirmations-fwd.hh \
	resolution.hh resolution-fwd.hh \
	resolutions_by_resolvent.hh resolutions_by_resolvent-fwd.hh \
	resolved.hh resolved-fwd.hh \
	resolvent.hh resolvent-fwd.hh \
	resolver.hh resolver-fwd.hh \
	resolver_functions.hh resolver_functions-fwd.hh \
	same_slot.hh same_slot-fwd.hh \
	sanitised_dependencies.hh sanitised_dependencies-fwd.hh \
	slot_name_or_null.hh slot_name_or_null-fwd.hh \
	spec_rewriter.hh spec_rewriter-fwd.hh \
	strongly_connected_component.hh strongly_connected_component-fwd.hh \
	suggest_restart.hh suggest_restart-fwd.hh \
	unsuitable_candidates.hh unsuitable_candidates-fwd.hh \
	use_existing.hh use_existing-fwd.hh use_existing-se.hh \
	why_changed_choices.hh why_changed_choices-fwd.hh

libpaludisresolver_a_SOURCES = \
	accumulate_deps_and_provides.cc \
	allow_choice_changes_helper.cc \
	allowed_to_remove_helper.cc \
	allowed_to_restart_helper.cc \
	always_via_binary_helper.cc \
	any_child_score.cc \
	can_use_helper.cc \
	change_by_resolvent.cc \
	change_type.cc \
	collect_depped_upon.cc \
	collect_installed.cc \
	collect_provided.cc \
	collect_purges.cc \
	collect_world.cc \
	confirm_helper.cc \
	constraint.cc \
	decider.cc \
	decision.cc \
	decision_utils.cc \
	decisions.cc \
	destination.cc \
	destination_types.cc \
	destination_utils.cc \
	find_replacing_helper.cc \
	find_repository_for_helper.cc \
	get_constraints_for_dependent_helper.cc \
	get_constraints_for_purge_helper.cc \
	get_constraints_for_via_binary_helper.cc \
	get_destination_types_for_blocker_helper.cc \
	get_destination_types_for_error_helper.cc \
	get_initial_constraints_for_helper.cc \
	get_resolvents_for_helper.cc \
	get_use_existing_nothing_helper.cc \
	interest_in_spec_helper.cc \
	job.cc \
	job_list.cc \
	job_lists.cc \
	job_requirements.cc \
	job_state.cc \
	labels_classifier.cc \
	make_destination_filtered_generator_helper.cc \
	make_origin_filtered_generator_helper.cc \
	make_uninstall_blocker.cc \
	make_unmaskable_filter_helper.cc \
	match_qpns.cc \
	nag.cc \
	order_early_helper.cc \
	orderer.cc \
	orderer_notes.cc \
	package_or_block_dep_spec.cc \
	prefer_or_avoid_helper.cc \
	reason.cc \
	reason_utils.cc \
	remove_if_dependent_helper.cc \
	required_confirmations.cc \
	resolution.cc \
	resolutions_by_resolvent.cc \
	resolved.cc \
	resolvent.cc \
	resolver.cc \
	resolver_functions.cc \
	same_slot.cc \
	sanitised_dependencies.cc \
	slot_name_or_null.cc \
	spec_rewriter.cc \
	strongly_connected_component.cc \
	suggest_restart.cc \
	unsuitable_candidates.cc \
	use_existing.cc \
	why_changed_choices.cc

if ENABLE_VIRTUALS_REPOSITORY
virtuals_tests = resolver_TEST_virtuals
endif

noinst_LIBRARIES = libpaludisresolver.a

TESTS = \
	resolver_TEST_simple \
	resolver_TEST_serialisation \
	resolver_TEST_blockers \
	resolver_TEST_cycles \
	resolver_TEST_suggestions \
	resolver_TEST_any \
	resolver_TEST_errors \
	resolver_TEST_continue_on_failure \
	resolver_TEST_uninstalls \
	resolver_TEST_fetches \
	resolver_TEST_purges \
	$(virtuals_tests)

check_PROGRAMS = $(TESTS)

check_SCRIPTS = \
	resolver_TEST_blockers_setup.sh resolver_TEST_blockers_cleanup.sh \
	resolver_TEST_cycles_setup.sh resolver_TEST_cycles_cleanup.sh \
	resolver_TEST_serialisation_setup.sh resolver_TEST_serialisation_cleanup.sh \
	resolver_TEST_suggestions_setup.sh resolver_TEST_suggestions_cleanup.sh \
	resolver_TEST_simple_setup.sh resolver_TEST_simple_cleanup.sh \
	resolver_TEST_virtuals_setup.sh resolver_TEST_virtuals_cleanup.sh \
	resolver_TEST_any_setup.sh resolver_TEST_any_cleanup.sh \
	resolver_TEST_errors_setup.sh resolver_TEST_errors_cleanup.sh \
	resolver_TEST_continue_on_failure_setup.sh resolver_TEST_continue_on_failure_cleanup.sh \
	resolver_TEST_uninstalls_setup.sh resolver_TEST_uninstalls_cleanup.sh \
	resolver_TEST_fetches_setup.sh resolver_TEST_fetches_cleanup.sh \
	resolver_TEST_purges_setup.sh resolver_TEST_purges_cleanup.sh

check_LIBRARIES = libpaludisresolvertest.a

libpaludisresolvertest_a_SOURCES = \
	resolver_test.hh \
	resolver_test.cc

resolver_TEST_blockers_SOURCES = resolver_TEST_blockers.cc

resolver_TEST_blockers_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_blockers_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_cycles_SOURCES = resolver_TEST_cycles.cc

resolver_TEST_cycles_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_cycles_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_virtuals_SOURCES = resolver_TEST_virtuals.cc

resolver_TEST_virtuals_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_virtuals_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_suggestions_SOURCES = resolver_TEST_suggestions.cc

resolver_TEST_suggestions_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_suggestions_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_serialisation_SOURCES = resolver_TEST_serialisation.cc

resolver_TEST_serialisation_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_serialisation_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_simple_SOURCES = resolver_TEST_simple.cc

resolver_TEST_simple_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_simple_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_any_SOURCES = resolver_TEST_any.cc

resolver_TEST_any_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_any_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_errors_SOURCES = resolver_TEST_errors.cc

resolver_TEST_errors_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_errors_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_continue_on_failure_SOURCES = resolver_TEST_continue_on_failure.cc

resolver_TEST_continue_on_failure_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_continue_on_failure_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_uninstalls_SOURCES = resolver_TEST_uninstalls.cc

resolver_TEST_uninstalls_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_uninstalls_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_fetches_SOURCES = resolver_TEST_fetches.cc

resolver_TEST_fetches_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_fetches_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

resolver_TEST_purges_SOURCES = resolver_TEST_purges.cc

resolver_TEST_purges_LDADD = \
	libpaludisresolvertest.a \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	libpaludisresolver.a \
	$(DYNAMIC_LD_LIBS)

resolver_TEST_purges_CXXFLAGS = $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

use_existing-se.hh : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

use_existing-se.cc : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

destination_types-se.hh : destination_types.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/destination_types.se > $@ ; then rm -f $@ ; exit 1 ; fi

destination_types-se.cc : destination_types.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/destination_types.se > $@ ; then rm -f $@ ; exit 1 ; fi

any_child_score-se.hh : any_child_score.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/any_child_score.se > $@ ; then rm -f $@ ; exit 1 ; fi

any_child_score-se.cc : any_child_score.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/any_child_score.se > $@ ; then rm -f $@ ; exit 1 ; fi

change_type-se.hh : change_type.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/change_type.se > $@ ; then rm -f $@ ; exit 1 ; fi

change_type-se.cc : change_type.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/change_type.se > $@ ; then rm -f $@ ; exit 1 ; fi

resolver_functions-se.hh : resolver_functions.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/resolver_functions.se > $@ ; then rm -f $@ ; exit 1 ; fi

resolver_functions-se.cc : resolver_functions.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/resolver_functions.se > $@ ; then rm -f $@ ; exit 1 ; fi

job_requirements-se.hh : job_requirements.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/job_requirements.se > $@ ; then rm -f $@ ; exit 1 ; fi

job_requirements-se.cc : job_requirements.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/job_requirements.se > $@ ; then rm -f $@ ; exit 1 ; fi

nag-se.hh : nag.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/nag.se > $@ ; then rm -f $@ ; exit 1 ; fi

nag-se.cc : nag.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/nag.se > $@ ; then rm -f $@ ; exit 1 ; fi

