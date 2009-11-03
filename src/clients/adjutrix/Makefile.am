include $(top_srcdir)/misc/common-makefile.am

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
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

TESTS = version_TEST help_TEST find_stable_candidates_TEST args_from_environment_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS) \
	find_stable_candidates_TEST_setup.sh find_stable_candidates_TEST_cleanup.sh

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

