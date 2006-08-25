AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = .

man_pages = qualudis.1

if ENABLE_QA

bin_PROGRAMS = qualudis
noinst_PROGRAMS = man-qualudis
man_MANS = $(man_pages)

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

qualudis_LDADD = \
	$(top_builddir)/paludis/qa/libpaludisqa.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/libcolour.a \
	$(PCREPLUSPLUS_LIBS) \
	$(DYNAMIC_LD_LIBS)

INCLUDES = $(PCREPLUSPLUS_CFLAGS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_REPOSITORY_SO_DIR="$(top_builddir)/paludis/repositories" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

endif

qualudis.1 : man-qualudis
	./man-qualudis | tee $@ | sed -e 's/^/    /'

man_qualudis_SOURCES = \
	man_qualudis.cc \
	qualudis_command_line.hh \
	qualudis_command_line.cc

man_qualudis_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/src/libcolour.a \
	$(DYNAMIC_LD_LIBS)

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_pages)
MAINTAINERCLEANFILES = Makefile.in
EXTRA_DIST = $(man_pages)

