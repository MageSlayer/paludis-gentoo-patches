AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
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
	./man-adjutrix | tee $@ | sed -e 's/^/    /'

man_adjutrix_SOURCES = \
	man_adjutrix.cc \
	command_line.hh \
	command_line.cc

man_adjutrix_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/libcolour.a \
	$(DYNAMIC_LD_LIBS)

adjutrix_SOURCES = \
	command_line.hh                       command_line.cc      \
	find_stable_candidates.hh             find_stable_candidates.cc \
	find_dropped_keywords.hh              find_dropped_keywords.cc \
	find_reverse_deps.hh                  find_reverse_deps.cc \
	keywords_graph.hh                     keywords_graph.cc \
	adjutrix_environment.hh               adjutrix_environment.cc \
	display_profiles_use.hh               display_profiles_use.cc \
	display_default_system_resolution.hh  display_default_system_resolution.cc \
	adjutrix.cc

adjutrix_LDADD = \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/src/libcolour.a \
	$(DYNAMIC_LD_LIBS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST find_stable_candidates_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	find_stable_candidates-sr.hh find_stable_candidates-sr.cc find_stable_candidates.sr \
	find_dropped_keywords-sr.hh find_dropped_keywords-sr.cc find_dropped_keywords.sr \
	find_stable_candidates_TEST_setup.sh find_stable_candidates_TEST_cleanup.sh

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_MANS) find_stable_candidates-sr.hh find_stable_candidates-sr.cc
MAINTAINERCLEANFILES = Makefile.in
BUILT_SOURCES = \
	find_stable_candidates-sr.hh find_stable_candidates-sr.cc \
	find_dropped_keywords-sr.hh find_dropped_keywords-sr.cc

find_stable_candidates-sr.hh : find_stable_candidates.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/find_stable_candidates.sr > $@

find_stable_candidates-sr.cc : find_stable_candidates.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/find_stable_candidates.sr > $@

find_dropped_keywords-sr.hh : find_dropped_keywords.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/find_dropped_keywords.sr > $@

find_dropped_keywords-sr.cc : find_dropped_keywords.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/find_dropped_keywords.sr > $@


