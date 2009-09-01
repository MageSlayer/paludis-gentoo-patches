CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
MAINTAINERCLEANFILES = Makefile.in
DISTCLEANFILES = \
	use_installed-se.hh use_installed-se.cc
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
EXTRA_DIST = \
	use_installed-se.hh use_installed-se.cc use_installed.se
BUILT_SOURCES = \
	use_installed-se.hh use_installed-se.cc

TESTS =

noinst_HEADERS = \
	arrow.hh arrow-fwd.hh \
	constraint.hh constraint-fwd.hh \
	decision.hh decision-fwd.hh \
	destinations.hh destinations-fwd.hh \
	qpn_s.hh qpn_s-fwd.hh \
	reason.hh reason-fwd.hh \
	resolution.hh resolution-fwd.hh \
	resolutions.hh resolutions-fwd.hh \
	resolver.hh resolver-fwd.hh \
	resolver_functions.hh resolver_functions-fwd.hh \
	sanitised_dependencies.hh sanitised_dependencies-fwd.hh \
	serialise.hh serialise-fwd.hh serialise-impl.hh \
	suggest_restart.hh suggest_restart-fwd.hh \
	use_installed.hh use_installed-fwd.hh use_installed-se.hh

libpaludisresolver_a_SOURCES = \
	arrow.cc \
	constraint.cc \
	decision.cc \
	destinations.cc \
	qpn_s.cc \
	reason.cc \
	resolution.cc \
	resolutions.cc \
	resolver.cc \
	sanitised_dependencies.cc \
	serialise.cc \
	suggest_restart.cc \
	use_installed.cc

TESTS_ENVIRONMENT = env PALUDIS_OPTIONS="" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_DISTRIBUTION="gentoo" \
	bash $(top_srcdir)/test/run_test.sh

if ENABLE_NEW_RESOLVER
noinst_LIBRARIES = libpaludisresolver.a
endif

check_PROGRAMS = $(TESTS)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

DISTCHECK_DEPS = libpaludisresolver.a

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

use_installed-se.hh : use_installed.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/use_installed.se > $@ ; then rm -f $@ ; exit 1 ; fi

use_installed-se.cc : use_installed.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/use_installed.se > $@ ; then rm -f $@ ; exit 1 ; fi

