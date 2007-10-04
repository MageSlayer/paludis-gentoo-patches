AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DDATADIR=\"$(datadir)\" \
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
	stage_builder.cc \
	target_config.hh \
	target_config.cc

contrarius_LDADD = \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/environments/adapted/libpaludisadaptedenvironment.la \
	$(top_builddir)/paludis/environments/libpaludisenvironments.la \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/dep_list/libpaludisdeplist.la \
	$(top_builddir)/src/output/liboutput.a \
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
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	CONTRARIUS_OPTIONS="" \
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
	$(top_builddir)/paludis/args/libpaludisman.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_pages) $(noinst_DATA) \
	contrarius_stage_options-sr.hh contrarius_stage_options-sr.cc
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = \
	$(man_pages) \
	contrarius_stage_options.sr \
	contrarius_stage_options-sr.hh \
	contrarius_stage_options-sr.cc \
	default_config.txt

contrarius_datadir = $(datadir)/paludis/contrarius/
contrarius_data_DATA = default_config.txt

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done
