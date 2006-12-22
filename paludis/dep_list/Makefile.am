CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = dep_list-sr.hh dep_list-sr.cc uninstall_list-sr.hh uninstall_list-sr.cc
BUILT_SOURCES = $(DISTCLEANFILES)
MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBDIR=\"$(libdir)\"

libpaludisdeplist_la_SOURCES = \
	dep_list.cc       dep_list.hh \
	uninstall_list.cc uninstall_list.hh

libpaludisdeplist_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0
libpaludisdeplist_la_LIBADD = \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludis.la

if MONOLITHIC

noinst_LTLIBRARIES = libpaludisdeplist.la

else

lib_LTLIBRARIES = libpaludisdeplist.la

endif

paludis_dep_list_includedir = $(includedir)/paludis/dep_list
paludis_dep_list_include_HEADERS = dep_list.hh uninstall_list.hh dep_list-sr.hh uninstall_list-sr.hh

EXTRA_DIST = \
	dep_list_TEST.cc \
	uninstall_list_TEST.cc \
	dep_list.sr dep_list-sr.hh dep_list-sr.cc \
	uninstall_list.sr uninstall_list-sr.hh uninstall_list-sr.cc

TESTS = \
	dep_list_TEST \
	uninstall_list_TEST

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)

dep_list_TEST_SOURCES = dep_list_TEST.cc
dep_list_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	libpaludisdeplist.la

uninstall_list_TEST_SOURCES = uninstall_list_TEST.cc
uninstall_list_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environment/test/libpaludistestenvironment.la \
	$(top_builddir)/paludis/repositories/fake/libpaludisfakerepository.la \
	$(top_builddir)/paludis/repositories/virtuals/libpaludisvirtualsrepository.la \
	libpaludisdeplist.la

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

dep_list-sr.hh : dep_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/dep_list.sr > $@

dep_list-sr.cc : dep_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/dep_list.sr > $@

uninstall_list-sr.hh : uninstall_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/uninstall_list.sr > $@

uninstall_list-sr.cc : uninstall_list.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/uninstall_list.sr > $@

