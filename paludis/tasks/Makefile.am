CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DDATADIR=\"$(datadir)\"
SUBDIRS = .

TESTS =

TESTS_ENVIRONMENT = env \
	PALUDIS_EBUILD_DIR="$(top_srcdir)/ebuild/" \
	PALUDIS_SKIP_CONFIG="yes" \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	bash $(top_srcdir)/test/run_test.sh

check_PROGRAMS = $(TESTS)
check_SCRIPTS =

lib_LIBRARIES = libpaludistasks.a
paludis_tasks_includedir = $(includedir)/paludis/tasks
paludis_tasks_include_HEADERS = \
	install_task.hh \
	uninstall_task.hh \
	sync_task.hh

libpaludistasks_a_SOURCES = $(paludis_tasks_include_HEADERS) \
	install_task.cc \
	uninstall_task.cc \
	sync_task.cc

