AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\" \
	-DLIBDIR=\"$(libdir)\"
SUBDIRS = .

man_pages = contrarius.1

bin_PROGRAMS = contrarius
noinst_PROGRAMS = man-contrarius
noinst_DATA = contrarius.html
man_MANS = $(man_pages)

contrarius_SOURCES = \
	command_line.hh \
	command_line.cc \
	contrarius.cc \
	install.hh \
	install.cc \
	stage.hh \
	stage.cc \
	stage_builder.hh \
	stage_builder.cc

contrarius_LDADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/default/libpaludisdefaultenvironment.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.a \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

contrarius_stage_options-sr.hh : contrarius_stage_options.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --header $(srcdir)/contrarius_stage_options.sr > $@

contrarius_stage_options-sr.cc : contrarius_stage_options.sr $(top_srcdir)/misc/make_sr.bash
	$(top_srcdir)/misc/make_sr.bash --source $(srcdir)/contrarius_stage_options.sr > $@

BUILT_SOURCES = \
	contrarius_stage_options-sr.hh \
	contrarius_stage_options-sr.cc

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./contrarius --version" > $@

contrarius.1 : man-contrarius
	./man-contrarius > $@

contrarius.html : man-contrarius
	./man-contrarius --html > $@

man_contrarius_SOURCES = \
	man_contrarius.cc \
	command_line.hh \
	command_line.cc

man_contrarius_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_pages) $(noinst_DATA) \
	contrarius_stage_options-sr.hh contrarius_stage_options-sr.cc
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = \
	$(man_pages) \
	contrarius_stage_options.sr \
	contrarius_stage_options-sr.hh \
	contrarius_stage_options-sr.cc

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done
