AM_CXXFLAGS = -I$(top_srcdir) @PALUDIS_CXXFLAGS@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DBIGTEMPDIR=\"/var/tmp\"
SUBDIRS = .

if ENABLE_QA

bin_PROGRAMS = qualudis
noinst_PROGRAMS = man-qualudis
man_MANS = qualudis.1

qualudis.1 : man-qualudis
	./man-qualudis | tee $@ | sed -e 's/^/    /'

man_qualudis_SOURCES = \
	man_qualudis.cc \
	qualudis_command_line.hh \
	qualudis_command_line.cc

man_qualudis_LDADD = \
	$(top_builddir)/paludis/qa/libpaludisqa.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

qualudis_SOURCES = \
	qualudis_command_line.hh      qualudis_command_line.cc      \
	qualudis.cc

qualudis_LDADD = \
	$(top_builddir)/paludis/repositories/nothing/libpaludisnothingrepository.la \
	$(top_builddir)/paludis/repositories/portage/libpaludisportagerepository.la \
	$(top_builddir)/paludis/repositories/vdb/libpaludisvdbrepository.la \
	$(top_builddir)/paludis/qa/libpaludisqa.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la

TESTS_ENVIRONMENT = env TEST_SCRIPT_DIR="$(srcdir)/" bash $(top_srcdir)/test/run_test.sh bash
TESTS = version_TEST

version_TEST :
	echo -e "#!/bin/sh\n./qualudis --version" > $@

endif

CLEANFILES = *~ version_TEST gmon.out *.gcov *.gcno *.gcda $(man_MANS)
MAINTAINERCLEANFILES = Makefile.in

