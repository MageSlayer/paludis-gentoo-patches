AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = .

man_pages = qualudis.1
noinst_DATA = qualudis.html

if ENABLE_QA

bin_PROGRAMS = qualudis
noinst_PROGRAMS = man-qualudis
man_MANS = $(man_pages)

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

if MONOLITHIC

qualudis_LDADD = \
	$(top_builddir)/paludis/qa/libpaludisqa.la \
	$(top_builddir)/paludis/environment/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/paludis/repositories/libpaludisrepositories.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/libxml/libpaludislibxml.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(PCREPLUSPLUS_LIBS) \
	$(LIBXML2DEPS_LIBS) \
	$(DYNAMIC_LD_LIBS)

else

qualudis_LDADD = \
	$(top_builddir)/paludis/qa/libpaludisqa.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(PCREPLUSPLUS_LIBS) \
	$(LIBXML2DEPS_LIBS) \
	$(DYNAMIC_LD_LIBS)

endif

INCLUDES = $(PCREPLUSPLUS_CFLAGS) $(LIBXML2DEPS_CFLAGS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

endif

qualudis.1 : man-qualudis
	./man-qualudis > $@

qualudis.html : man-qualudis
	./man-qualudis --html > $@

man_qualudis_SOURCES = \
	man_qualudis.cc \
	qualudis_command_line.hh \
	qualudis_command_line.cc

man_qualudis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_pages) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = $(man_pages)

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done


