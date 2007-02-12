CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda

DISTCLEANFILES = \
	merger-sr.hh merger-sr.cc merger-se.hh merger-se.cc

BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_merger_includedir = $(includedir)/paludis/merger
paludis_merger_include_HEADERS = \
	merger.hh \
	merger-sr.hh \
	merger-se.cc

libpaludismerger_la_SOURCES = \
	merger.cc merger.hh

libpaludismerger_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludismerger_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

lib_LTLIBRARIES = libpaludismerger.la

EXTRA_DIST = \
	merger_TEST.cc merger_TEST_setup.sh merger_TEST_cleanup.sh \
	merger-sr.hh merger-sr.cc merger.sr \
	merger-se.hh merger-se.cc merger.se

TESTS = \
	merger_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/gentoo/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = merger_TEST_setup.sh merger_TEST_cleanup.sh

merger_TEST_SOURCES = merger_TEST.cc
merger_TEST_LDADD = \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludismerger.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

merger-sr.hh : merger.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $(srcdir)/merger.sr > $@ ; then rm -f $@ ; exit 1 ; fi

merger-sr.cc : merger.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $(srcdir)/merger.sr > $@ ; then rm -f $@ ; exit 1 ; fi

merger-se.hh : merger.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/merger.se > $@ ; then rm -f $@ ; exit 1 ; fi

merger-se.cc : merger.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/merger.se > $@ ; then rm -f $@ ; exit 1 ; fi

