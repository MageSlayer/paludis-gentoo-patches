include $(top_srcdir)/misc/common-makefile.am

AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = inquisitio
noinst_PROGRAMS = man-inquisitio
man_MANS = inquisitio.1

inquisitio.1 : man-inquisitio
	./man-inquisitio > $@

man_inquisitio_SOURCES = \
	man_inquisitio.cc \
	command_line.hh       command_line.cc

man_inquisitio_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisman_@PALUDIS_PC_SLOT@.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludismanpagethings_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS) \
	$(PCRECPPDEPS_LIBS)

inquisitio_SOURCES = \
	command_line.hh                 command_line.cc \
	matcher.hh                      matcher.cc \
	pcre_matcher.hh                 pcre_matcher.cc \
	text_matcher.hh                 text_matcher.cc \
	exact_matcher.hh                exact_matcher.cc \
	do_search.hh                    do_search.cc \
	query_task.hh                   query_task.cc \
	extractor.hh                    extractor.cc \
	name_description_extractor.hh   name_description_extractor.cc \
	key_extractor.hh                key_extractor.cc \
	inquisitio.cc

inquisitio_LDADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/args/libpaludisargs_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS) \
	$(PCRECPPDEPS_LIBS)

inquisitio_CXXFLAGS = $(AM_CXXFLAGS) $(PCRECPPDEPS_CFLAGS)

TESTS = version_TEST help_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS)

DISTCLEANFILES = $(man_MANS) $(noinst_DATA)

