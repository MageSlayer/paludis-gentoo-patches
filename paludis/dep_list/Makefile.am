CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = \
	dep_list-sr.hh dep_list-sr.cc \
	uninstall_list-sr.hh uninstall_list-sr.cc \
	options-se.hh options-se.cc
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

paludis_dep_list_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/dep_list
paludis_dep_list_include_HEADERS = \
	dep_list.hh \
	query_visitor.hh \
	show_suggest_visitor.hh \
	uninstall_list.hh \
	condition_tracker.hh \
	exceptions.hh \
	options.hh \
	dep_list-sr.hh \
	dep_list-fwd.hh \
	uninstall_list-sr.hh \
	range_rewriter.hh \
	options-se.hh

libpaludisdeplist_la_SOURCES = \
	options.cc        options.hh \
	exceptions.cc     exceptions.hh \
	dep_list.cc       dep_list.hh           dep_list-fwd.hh \
	uninstall_list.cc uninstall_list.hh \
	range_rewriter.cc range_rewriter.hh \
	query_visitor.cc  query_visitor.hh \
	show_suggest_visitor.cc show_suggest_visitor.hh \
	condition_tracker.cc condition_tracker.hh

libpaludisdeplist_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludisdeplist_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

lib_LTLIBRARIES = libpaludisdeplist.la

EXTRA_DIST = \
	dep_list_TEST.cc \
	dep_list_TEST.hh \
	dep_list_TEST_blockers.cc \
	uninstall_list_TEST.cc \
	range_rewriter_TEST.cc \
	dep_list.sr dep_list-sr.hh dep_list-sr.cc \
	uninstall_list.sr uninstall_list-sr.hh uninstall_list-sr.cc \
	options.se options-se.hh options-se.cc

TESTS = \
	dep_list_TEST \
	dep_list_TEST_blockers \
	uninstall_list_TEST \
	range_rewriter_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/gentoo/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	LD_LIBRARY_PATH="`echo $$LD_LIBRARY_PATH: | sed -e 's,^:,,'`` \
		$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/`:` \
		$(top_srcdir)/paludis/repositories/gentoo/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/gentoo/.libs/`" \
	bash $(top_srcdir)/test/run_test.sh


check_PROGRAMS = $(TESTS)

dep_list_TEST_SOURCES = dep_list_TEST.cc
dep_list_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisdeplist.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

dep_list_TEST_blockers_SOURCES = dep_list_TEST_blockers.cc
dep_list_TEST_blockers_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisdeplist.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

uninstall_list_TEST_SOURCES = uninstall_list_TEST.cc
uninstall_list_TEST_LDADD = \
	$(top_builddir)/paludis/environments/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisdeplist.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

range_rewriter_TEST_SOURCES = range_rewriter_TEST.cc
range_rewriter_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	libpaludisdeplist.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

dep_list-sr.hh : dep_list.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $(srcdir)/dep_list.sr > $@ ; then rm -f $@ ; exit 1 ; fi

dep_list-sr.cc : dep_list.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $(srcdir)/dep_list.sr > $@ ; then rm -f $@ ; exit 1 ; fi

uninstall_list-sr.hh : uninstall_list.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --header $(srcdir)/uninstall_list.sr > $@ ; then rm -f $@ ; exit 1 ; fi

uninstall_list-sr.cc : uninstall_list.sr $(top_srcdir)/misc/make_sr.bash
	if ! $(top_srcdir)/misc/make_sr.bash --source $(srcdir)/uninstall_list.sr > $@ ; then rm -f $@ ; exit 1 ; fi

options-se.hh : options.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/options.se > $@ ; then rm -f $@ ; exit 1 ; fi

options-se.cc : options.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/options.se > $@ ; then rm -f $@ ; exit 1 ; fi

