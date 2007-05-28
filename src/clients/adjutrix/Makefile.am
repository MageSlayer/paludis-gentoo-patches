AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = adjutrix
noinst_PROGRAMS = man-adjutrix
noinst_DATA = adjutrix.html

man_MANS = adjutrix.1

adjutrix.1 : man-adjutrix
	./man-adjutrix > $@

adjutrix.html : man-adjutrix
	./man-adjutrix --html > $@

man_adjutrix_SOURCES = \
	man_adjutrix.cc \
	command_line.hh \
	command_line.cc

man_adjutrix_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/args/libpaludisman.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludismanpagethings.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

adjutrix_SOURCES = \
	command_line.hh                       command_line.cc      \
	find_stable_candidates.hh             find_stable_candidates.cc \
	find_dropped_keywords.hh              find_dropped_keywords.cc \
	find_insecure_packages.hh             find_insecure_packages.cc \
	find_reverse_deps.hh                  find_reverse_deps.cc \
	find_unused_packages.hh               find_unused_packages.cc \
	keywords_graph.hh                     keywords_graph.cc \
	display_profiles_use.hh               display_profiles_use.cc \
	display_default_system_resolution.hh  display_default_system_resolution.cc \
	what_needs_keywording.hh              what_needs_keywording.cc \
	downgrade_check.hh                    downgrade_check.cc \
	adjutrix.cc

adjutrix_LDADD = \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	ADJUTRIX_OPTIONS="" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST find_stable_candidates_TEST args_from_environment_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	find_stable_candidates-sr.hh find_stable_candidates-sr.cc find_stable_candidates.sr \
	find_dropped_keywords-sr.hh find_dropped_keywords-sr.cc find_dropped_keywords.sr \
	downgrade_check-sr.hh downgrade_check-sr.cc downgrade_check.sr \
	find_stable_candidates_TEST_setup.sh find_stable_candidates_TEST_cleanup.sh

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_MANS) $(noinst_DATA) \
	find_stable_candidates-sr.hh find_stable_candidates-sr.cc
MAINTAINERCLEANFILES = Makefile.in
BUILT_SOURCES = \
	find_stable_candidates-sr.hh find_stable_candidates-sr.cc \
	find_dropped_keywords-sr.hh find_dropped_keywords-sr.cc \
	downgrade_check-sr.hh downgrade_check-sr.cc

find_stable_candidates-sr.hh : find_stable_candidates.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/find_stable_candidates.sr > $@

find_stable_candidates-sr.cc : find_stable_candidates.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/find_stable_candidates.sr > $@

find_dropped_keywords-sr.hh : find_dropped_keywords.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/find_dropped_keywords.sr > $@

find_dropped_keywords-sr.cc : find_dropped_keywords.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/find_dropped_keywords.sr > $@

downgrade_check-sr.hh : downgrade_check.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/downgrade_check.sr > $@

downgrade_check-sr.cc : downgrade_check.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/downgrade_check.sr > $@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


