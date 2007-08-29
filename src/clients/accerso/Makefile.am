AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src \
	      @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@
DEFS= \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	-DLIBDIR=\"$(libdir)\"

SUBDIRS = .

bin_PROGRAMS = accerso
noinst_PROGRAMS = man-accerso
noinst_DATA = accerso.html
man_MANS = accerso.1

accerso.1 : man-accerso
	./man-accerso > $@

accerso.html : man-accerso
	./man-accerso --html > $@

man_accerso_SOURCES = \
	man_accerso.cc \
	command_line.hh \
	command_line.cc

man_accerso_LDADD = \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/args/libpaludisman.a \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/libpaludismanpagethings.la \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(top_builddir)/src/output/liboutput.a \
	$(DYNAMIC_LD_LIBS)

accerso_SOURCES = \
	command_line.hh      command_line.cc      \
	accerso.cc

accerso_LDADD = \
	$(top_builddir)/paludis/tasks/libpaludistasks.la \
	$(top_builddir)/paludis/libpaludis.la \
	$(top_builddir)/paludis/args/libpaludisargs.la \
	$(top_builddir)/paludis/util/libpaludisutil.la \
	$(top_builddir)/paludis/environments/no_config/libpaludisnoconfigenvironment.la \
	$(top_builddir)/src/output/liboutput.a \
	$(top_builddir)/src/common_args/libcommonargs.a \
	$(DYNAMIC_LD_LIBS)

TESTS_ENVIRONMENT = env \
	TEST_SCRIPT_DIR="$(srcdir)/" \
	PALUDIS_NO_GLOBAL_HOOKS="yes" \
	PALUDIS_NO_XTERM_TITLES="yes" \
	PALUDIS_OPTIONS="" \
	PALUDIS_EBUILD_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_srcdir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EBUILD_DIR_FALLBACK="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories/e/ebuild/`" \
	PALUDIS_EAPIS_DIR="$(top_srcdir)/paludis/repositories/e/eapis/" \
	PALUDIS_DISTRIBUTIONS_DIR="$(top_srcdir)/paludis/distributions/" \
	PALUDIS_DISTRIBUTION="gentoo" \
	PALUDIS_REPOSITORY_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/repositories`" \
	PALUDIS_ENVIRONMENT_SO_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/environments`" \
	PALUDIS_NO_CHOWN="yupyup" \
	PALUDIS_REDUCED_USERNAME="`id -un`" \
	PALUDIS_OUTPUTWRAPPER_DIR="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/`" \
	TEST_OUTPUT_WRAPPER="`$(top_srcdir)/paludis/repositories/e/ebuild/utils/canonicalise $(top_builddir)/paludis/util/outputwrapper`" \
	SYSCONFDIR="$(sysconfdir)" \
	bash $(top_srcdir)/test/run_test.sh bash

TESTS = version_TEST \
	help_TEST

EXTRA_DIST = \
	$(man_MANS) \
	$(TESTS)

CLEANFILES = *~ gmon.out *.gcov *.gcno *.gcda
DISTCLEANFILES = $(man_MANS) $(noinst_DATA)
MAINTAINERCLEANFILES = Makefile.in

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

