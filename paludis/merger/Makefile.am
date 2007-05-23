CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda

DISTCLEANFILES = \
	merger-sr.hh merger-sr.cc entry_type-se.hh entry_type-se.cc \
	unmerger-sr.hh unmerger-sr.cc

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
	entry_type.hh \
	merger.hh \
	merger-sr.hh \
	unmerger.hh \
	unmerger-sr.hh

libpaludismerger_la_SOURCES = \
	entry_type.cc entry_type.hh \
	merger.cc merger.hh \
	unmerger.cc unmerger.hh

libpaludismerger_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludismerger_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/selinux/libpaludisselinux.la

lib_LTLIBRARIES = libpaludismerger.la

EXTRA_DIST = \
	merger_TEST.cc merger_TEST_setup.sh merger_TEST_cleanup.sh \
	merger-sr.hh merger-sr.cc merger.sr \
	entry_type-se.hh entry_type-se.cc entry_type.se \
	unmerger-sr.hh unmerger-sr.cc unmerger.sr

TESTS = \
	merger_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/paludis/repositories/gentoo/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_HOOKER_DIR="$(top_srcdir)/paludis/" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS = merger_TEST_setup.sh merger_TEST_cleanup.sh

merger_TEST_SOURCES = merger_TEST.cc
merger_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
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

unmerger-sr.hh : unmerger.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $(srcdir)/unmerger.sr > $@ ; then rm -f $@ ; exit 1 ; fi

unmerger-sr.cc : unmerger.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $(srcdir)/unmerger.sr > $@ ; then rm -f $@ ; exit 1 ; fi

entry_type-se.hh : entry_type.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/entry_type.se > $@ ; then rm -f $@ ; exit 1 ; fi

entry_type-se.cc : entry_type.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/entry_type.se > $@ ; then rm -f $@ ; exit 1 ; fi

