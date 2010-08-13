include $(top_srcdir)/misc/common-makefile.am

DISTCLEANFILES = \
	dep_list_options-se.hh dep_list_options-se.cc \
	install_task-se.hh install_task-se.cc \
	uninstall_task-se.hh uninstall_task-se.cc

AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@

EXTRA_DIST = \
	dep_list_options-se.hh dep_list_options-se.cc dep_list_options.se \
	install_task-se.hh install_task-se.cc install_task.se \
	uninstall_task-se.hh uninstall_task-se.cc uninstall_task.se

BUILT_SOURCES = \
	dep_list_options-se.hh dep_list_options-se.cc \
	install_task-se.hh install_task-se.cc \
	uninstall_list-se.hh uninstall_list-se.cc

paludis_legacy_includedir = $(includedir)/paludis-$(PALUDIS_PC_SLOT)/paludis/legacy

paludis_legacy_include_HEADERS = \
	dep_list.hh dep_list-fwd.hh \
	dep_list_exceptions.hh \
	dep_list_options.hh dep_list_options-se.hh \
	find_unused_packages_task.hh \
	handled_information.hh handled_information-fwd.hh \
	install_task.hh install_task-se.hh \
	query_visitor.hh \
	range_rewriter.hh \
	report_task.hh \
	show_suggest_visitor.hh \
	sync_task.hh \
	tasks_exceptions.hh \
	uninstall_list.hh uninstall_list-se.hh \
	uninstall_task.hh

libpaludislegacy_@PALUDIS_PC_SLOT@_la_SOURCES = \
	dep_list.cc \
	dep_list_exceptions.cc \
	dep_list_options.cc \
	find_unused_packages_task.cc \
	handled_information.cc \
	install_task.cc \
	query_visitor.cc \
	range_rewriter.cc \
	report_task.cc \
	show_suggest_visitor.cc \
	sync_task.cc \
	tasks_exceptions.cc \
	uninstall_list.cc \
	uninstall_task.cc

libpaludislegacy_@PALUDIS_PC_SLOT@_la_LDFLAGS = -version-info @VERSION_LIB_CURRENT@:@VERSION_LIB_REVISION@:0

libpaludislegacy_@PALUDIS_PC_SLOT@_la_LIBADD = \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

lib_LTLIBRARIES = libpaludislegacy_@PALUDIS_PC_SLOT@.la

TESTS = \
	dep_list_TEST \
	dep_list_TEST_blockers \
	range_rewriter_TEST \
	uninstall_list_TEST

check_PROGRAMS = $(TESTS)

dep_list_TEST_SOURCES = dep_list_TEST.cc dep_list_TEST.hh

dep_list_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/legacy/libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

dep_list_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

dep_list_TEST_blockers_SOURCES = dep_list_TEST_blockers.cc dep_list_TEST.hh

dep_list_TEST_blockers_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/legacy/libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

dep_list_TEST_blockers_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

range_rewriter_TEST_SOURCES = range_rewriter_TEST.cc

range_rewriter_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/legacy/libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

range_rewriter_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

uninstall_list_TEST_SOURCES = uninstall_list_TEST.cc

uninstall_list_TEST_LDADD = \
	$(top_builddir)/paludis/util/test_extras.o \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/legacy/libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/libpaludis_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la \
	$(DYNAMIC_LD_LIBS)

uninstall_list_TEST_CXXFLAGS = -I$(top_srcdir) $(AM_CXXFLAGS) @PALUDIS_CXXFLAGS_NO_DEBUGGING@

test_ldadd = \
	$(top_builddir)/paludis/util/test_extras.o \
	libpaludislegacy_@PALUDIS_PC_SLOT@.la \
	$(top_builddir)/test/libtest.a \
	$(top_builddir)/paludis/util/libpaludisutil_@PALUDIS_PC_SLOT@.la

dep_list_options-se.hh : dep_list_options.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/dep_list_options.se > $@ ; then rm -f $@ ; exit 1 ; fi

dep_list_options-se.cc : dep_list_options.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/dep_list_options.se > $@ ; then rm -f $@ ; exit 1 ; fi

uninstall_list-se.hh : uninstall_list.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/uninstall_list.se > $@ ; then rm -f $@ ; exit 1 ; fi

uninstall_list-se.cc : uninstall_list.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/uninstall_list.se > $@ ; then rm -f $@ ; exit 1 ; fi

install_task-se.hh : install_task.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --header $(srcdir)/install_task.se > $@ ; then rm -f $@ ; exit 1 ; fi

install_task-se.cc : install_task.se $(top_srcdir)/misc/make_se.bash
	if ! $(top_srcdir)/misc/make_se.bash --source $(srcdir)/install_task.se > $@ ; then rm -f $@ ; exit 1 ; fi

