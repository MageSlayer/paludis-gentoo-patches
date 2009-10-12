AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = adjutrix
noinst_PROGRAMS = man-adjutrix

man_MANS = adjutrix.1

adjutrix.1 : man-adjutrix
	./man-adjutrix > $@

man_adjutrix_SOURCES = \
	man_adjutrix.cc \
	command_line.hh \
	command_line.cc

man_adjutrix_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

adjutrix_SOURCES = \
	command_line.hh                       command_line.cc      \
	find_stable_candidates.hh             find_stable_candidates.cc \
	find_dropped_keywords.hh              find_dropped_keywords.cc \
	find_insecure_packages.hh             find_insecure_packages.cc \
	find_reverse_deps.hh                  find_reverse_deps.cc \
	find_unused_packages.hh               find_unused_packages.cc \
	keywords_graph.hh                     keywords_graph.cc \
	display_default_system_resolution.hh  display_default_system_resolution.cc \
	what_needs_keywording.hh              what_needs_keywording.cc \
	downgrade_check.hh                    downgrade_check.cc \
	adjutrix.cc

adjutrix_LDADD = \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EXTRA_EBUILD_MODULES_DIRS="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_SUFFIXES_FILE="$(top_srcdir)/paludis/repositories/e/ebuild_entries_suffixes.conf" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_OPTIONS="" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_DEFAULT_OUTPUT_CONF="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/tests_output.conf`" \
	PALUDIS_OUTPUT_MANAGERS_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/environments/paludis/output_managers/`" \
	SYSCONFDIR="$(sysconfdir)" \
	ADJUTRIX_OPTIONS="" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST find_stable_candidates_TEST args_from_environment_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	find_stable_candidates_TEST_setup.sh find_stable_candidates_TEST_cleanup.sh

CLEANFILES = *~ gmon.out *.gcov *.gcno  *.gcda *.loT *.epicfail
DISTCLEANFILES = $(man_MANS) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

DISTCHECK_DEPS = adjutrix.1

distcheck-deps-local : $(DISTCHECK_DEPS)

distcheck-deps : distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in $(SUBDIRS) . ; do if test x$$s = x. ; then $(MAKE) distcheck-deps-local || exit 1 ; \
	    else $(MAKE) -C $$s distcheck-deps || exit 1 ; fi ; done

