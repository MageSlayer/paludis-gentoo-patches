include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES = \
	any_child_score-se.hh any_child_score-se.cc \
	change_type-se.hh change_type-se.cc \
	destination_types-se.hh destination_types-se.cc \
	failure_kinds-se.hh failure_kinds-se.cc \
	resolver_functions-se.hh resolver_functions-se.cc \
	use_existing-se.hh use_existing-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = \
	any_child_score-se.hh any_child_score-se.cc any_child_score.se \
	change_type-se.hh change_type-se.cc change_type.se \
	destination_types-se.hh destination_types-se.cc destination_types.se \
	failure_kinds-se.hh failure_kinds-se.cc failure_kinds.se \
	resolver_functions-se.hh resolver_functions-se.cc resolver_functions.se \
	use_existing-se.hh use_existing-se.cc use_existing.se \
	$(check_SCRIPTS)
BUILT_SOURCES = \
	any_child_score-se.hh any_child_score-se.cc \
	change_type-se.hh change_type-se.cc \
	destination_types-se.hh destination_types-se.cc \
	failure_kinds-se.hh failure_kinds-se.cc \
	resolver_functions-se.hh resolver_functions-se.cc \
	use_existing-se.hh use_existing-se.cc

noinst_HEADERS = \
	any_child_score.hh any_child_score-fwd.hh any_child_score-se.hh \
	arrow.hh arrow-fwd.hh \
	change_type.hh change_type-fwd.hh change_type-se.hh \
	constraint.hh constraint-fwd.hh \
	decider.hh decider-fwd.hh \
	decision.hh decision-fwd.hh \
	destination.hh destination-fwd.hh \
	destination_types.hh destination_types-fwd.hh destination_types-se.hh \
	failure_kinds.hh failure_kinds-fwd.hh failure_kinds-se.hh \
	job.hh job-fwd.hh \
	job_id.hh job_id-fwd.hh \
	job_state.hh job_state-fwd.hh \
	jobs.hh jobs-fwd.hh \
	orderer.hh orderer-fwd.hh \
	package_or_block_dep_spec.hh package_or_block_dep_spec-fwd.hh \
	reason.hh reason-fwd.hh \
	required_confirmations.hh required_confirmations-fwd.hh \
	resolution.hh resolution-fwd.hh \
	resolutions.hh resolutions-fwd.hh \
	resolvent.hh resolvent-fwd.hh \
	resolver.hh resolver-fwd.hh \
	resolver_functions.hh resolver_functions-fwd.hh \
	resolver_lists.hh resolver_lists-fwd.hh \
	sanitised_dependencies.hh sanitised_dependencies-fwd.hh \
	slot_name_or_null.hh slot_name_or_null-fwd.hh \
	spec_rewriter.hh spec_rewriter-fwd.hh \
	suggest_restart.hh suggest_restart-fwd.hh \
	unsuitable_candidates.hh unsuitable_candidates-fwd.hh \
	use_existing.hh use_existing-fwd.hh use_existing-se.hh

libpaludisresolver_a_SOURCES = \
	any_child_score.cc \
	arrow.cc \
	change_type.cc \
	constraint.cc \
	decider.cc \
	decision.cc \
	destination.cc \
	destination_types.cc \
	failure_kinds.cc \
	job.cc \
	job_id.cc \
	job_state.cc \
	jobs.cc \
	orderer.cc \
	package_or_block_dep_spec.cc \
	reason.cc \
	required_confirmations.cc \
	resolution.cc \
	resolutions.cc \
	resolvent.cc \
	resolver.cc \
	resolver_functions.cc \
	resolver_lists.cc \
	sanitised_dependencies.cc \
	slot_name_or_null.cc \
	spec_rewriter.cc \
	suggest_restart.cc \
	unsuitable_candidates.cc \
	use_existing.cc

if ENABLE_VIRTUALS_REPOSITORY
virtuals_tests = resolver_TEST_virtuals
endif

if ENABLE_NEW_RESOLVER
noinst_LIBRARIES = libpaludisresolver.a

TESTS = \
	resolver_TEST_simple \
	resolver_TEST_serialisation \
	resolver_TEST_blockers \
	resolver_TEST_cycles \
	resolver_TEST_suggestions \
	resolver_TEST_any \
	resolver_TEST_errors \
	$(virtuals_tests)
endif


check_PROGRAMS = $(TESTS)
check_SCRIPTS = \
	resolver_TEST_blockers_setup.sh resolver_TEST_blockers_cleanup.sh \
	resolver_TEST_cycles_setup.sh resolver_TEST_cycles_cleanup.sh \
	resolver_TEST_serialisation_setup.sh resolver_TEST_serialisation_cleanup.sh \
	resolver_TEST_suggestions_setup.sh resolver_TEST_suggestions_cleanup.sh \
	resolver_TEST_simple_setup.sh resolver_TEST_simple_cleanup.sh \
	resolver_TEST_virtuals_setup.sh resolver_TEST_virtuals_cleanup.sh \
	resolver_TEST_any_setup.sh resolver_TEST_any_cleanup.sh \
	resolver_TEST_errors_setup.sh resolver_TEST_errors_cleanup.sh
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

use_existing-se.hh : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

use_existing-se.cc : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

failure_kinds-se.hh : failure_kinds.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/failure_kinds.se > $@ ; then rm -f $@ ; exit 1 ; fi

failure_kinds-se.cc : failure_kinds.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/failure_kinds.se > $@ ; then rm -f $@ ; exit 1 ; fi

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

