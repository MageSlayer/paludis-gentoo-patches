AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = inquisitio
noinst_PROGRAMS = man-inquisitio
noinst_DATA = inquisitio.html
man_MANS = inquisitio.1

inquisitio.1 : man-inquisitio
	./man-inquisitio > $@

inquisitio.html : man-inquisitio
	./man-inquisitio --html > $@

man_inquisitio_SOURCES = \
	man_inquisitio.cc \
	command_line.hh       command_line.cc

man_inquisitio_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludismanpagethings.la \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS) \
	$(PCREPLUSPLUS_LIBS)

inquisitio_SOURCES = \
	command_line.hh              command_line.cc \
	matcher.hh                   matcher.cc \
	pcre_matcher.hh              pcre_matcher.cc \
	text_matcher.hh              text_matcher.cc \
	extractor.hh                 extractor.cc \
	description_extractor.hh     description_extractor.cc \
	name_extractor.hh            name_extractor.cc \
	do_search.hh                 do_search.cc \
	query_task.hh                query_task.cc \
	inquisitio.cc

inquisitio_LDADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/default/libpaludisdefaultenvironment.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS) \
	$(PCREPLUSPLUS_LIBS)

inquisitio_CXXFLAGS = $(PCREPLUSPLUS_CFLAGS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_OPTIONS="" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_srcdir)/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/ebuild/`" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST help_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_MANS) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

