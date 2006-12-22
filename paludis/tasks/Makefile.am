CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = stage_options-sr.hh stage_options-sr.cc
MAINTAINERCLEANFILES = Makefile.in
AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
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
	sync_task.hh \
	stage_builder_task.hh \
	stage_options-sr.hh \
	report_task.hh

libpaludistasks_a_SOURCES = $(paludis_tasks_include_HEADERS) \
	install_task.cc \
	uninstall_task.cc \
	sync_task.cc \
	stage_builder_task.cc \
	report_task.cc

BUILT_SOURCES = \
	stage_options-sr.hh \
	stage_options-sr.cc

EXTRA_DIST = \
	stage_options.sr \
	stage_options-sr.hh \
	stage_options-sr.cc

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

stage_options-sr.hh : stage_options.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/stage_options.sr > $@

stage_options-sr.cc : stage_options.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/stage_options.sr > $@
