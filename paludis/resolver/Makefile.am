include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES = \
	destination_types-se.hh destination_types-se.cc \
	use_existing-se.hh use_existing-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = \
	destination_types-se.hh destination_types-se.cc destination_types.se \
	use_existing-se.hh use_existing-se.cc use_existing.se \
	$(check_SCRIPTS)
BUILT_SOURCES = \
	destination_types-se.hh destination_types-se.cc \
	use_existing-se.hh use_existing-se.cc

noinst_HEADERS = \
	arrow.hh arrow-fwd.hh \
	constraint.hh constraint-fwd.hh \
	decision.hh decision-fwd.hh \
	destination.hh destination-fwd.hh \
	destination_types.hh destination_types-fwd.hh destination_types-se.hh \
	reason.hh reason-fwd.hh \
	resolution.hh resolution-fwd.hh \
	resolutions.hh resolutions-fwd.hh \
	resolvent.hh resolvent-fwd.hh \
	resolver.hh resolver-fwd.hh \
	resolver_functions.hh resolver_functions-fwd.hh \
	sanitised_dependencies.hh sanitised_dependencies-fwd.hh \
	spec_rewriter.hh spec_rewriter-fwd.hh \
	suggest_restart.hh suggest_restart-fwd.hh \
	unsuitable_candidates.hh unsuitable_candidates-fwd.hh \
	use_existing.hh use_existing-fwd.hh use_existing-se.hh

libpaludisresolver_a_SOURCES = \
	arrow.cc \
	constraint.cc \
	decision.cc \
	destination.cc \
	destination_types.cc \
	reason.cc \
	resolution.cc \
	resolutions.cc \
	resolvent.cc \
	resolver.cc \
	sanitised_dependencies.cc \
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
	resolver_TEST_blockers \
	resolver_TEST_serialisation \
	resolver_TEST_suggestions \
	$(virtuals_tests)
endif


check_PROGRAMS = $(TESTS)
check_SCRIPTS = \
	resolver_TEST_blockers_setup.sh resolver_TEST_blockers_cleanup.sh \
	resolver_TEST_serialisation_setup.sh resolver_TEST_serialisation_cleanup.sh \
	resolver_TEST_suggestions_setup.sh resolver_TEST_suggestions_cleanup.sh \
	resolver_TEST_virtuals_setup.sh resolver_TEST_virtuals_cleanup.sh
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

use_existing-se.hh : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

use_existing-se.cc : use_existing.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/use_existing.se > $@ ; then rm -f $@ ; exit 1 ; fi

destination_types-se.hh : destination_types.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/destination_types.se > $@ ; then rm -f $@ ; exit 1 ; fi

destination_types-se.cc : destination_types.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/destination_types.se > $@ ; then rm -f $@ ; exit 1 ; fi

